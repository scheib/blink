/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "core/rendering/svg/RenderSVGResourceFilter.h"

#include "core/dom/ElementTraversal.h"
#include "core/svg/SVGFilterPrimitiveStandardAttributes.h"
#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/filters/SkiaImageFilterBuilder.h"
#include "platform/graphics/filters/SourceAlpha.h"
#include "platform/graphics/filters/SourceGraphic.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace blink {

void FilterData::trace(Visitor* visitor)
{
#if ENABLE(OILPAN)
    visitor->trace(filter);
    visitor->trace(builder);
#endif
}

RenderSVGResourceFilter::RenderSVGResourceFilter(SVGFilterElement* node)
    : RenderSVGResourceContainer(node)
{
}

RenderSVGResourceFilter::~RenderSVGResourceFilter()
{
}

void RenderSVGResourceFilter::trace(Visitor* visitor)
{
#if ENABLE(OILPAN)
    visitor->trace(m_filter);
#endif
    RenderSVGResourceContainer::trace(visitor);
}

void RenderSVGResourceFilter::destroy()
{
    m_filter.clear();
    RenderSVGResourceContainer::destroy();
}

bool RenderSVGResourceFilter::isChildAllowed(RenderObject* child, RenderStyle*) const
{
    return child->isSVGResourceFilterPrimitive();
}

void RenderSVGResourceFilter::removeAllClientsFromCache(bool markForInvalidation)
{
    m_filter.clear();
    markAllClientsForInvalidation(markForInvalidation ? LayoutAndBoundariesInvalidation : ParentOnlyInvalidation);
}

void RenderSVGResourceFilter::removeClientFromCache(RenderObject* client, bool markForInvalidation)
{
    ASSERT(client);

    m_filter.remove(client);

    markClientForInvalidation(client, markForInvalidation ? BoundariesInvalidation : ParentOnlyInvalidation);
}

PassRefPtrWillBeRawPtr<SVGFilterBuilder> RenderSVGResourceFilter::buildPrimitives(SVGFilter* filter)
{
    SVGFilterElement* filterElement = toSVGFilterElement(element());
    FloatRect targetBoundingBox = filter->targetBoundingBox();

    // Add effects to the builder
    RefPtrWillBeRawPtr<SVGFilterBuilder> builder = SVGFilterBuilder::create(SourceGraphic::create(filter), SourceAlpha::create(filter));
    for (SVGElement* element = Traversal<SVGElement>::firstChild(*filterElement); element; element = Traversal<SVGElement>::nextSibling(*element)) {
        if (!element->isFilterEffect() || !element->renderer())
            continue;

        SVGFilterPrimitiveStandardAttributes* effectElement = static_cast<SVGFilterPrimitiveStandardAttributes*>(element);
        RefPtrWillBeRawPtr<FilterEffect> effect = effectElement->build(builder.get(), filter);
        if (!effect) {
            builder->clearEffects();
            return nullptr;
        }
        builder->appendEffectToEffectReferences(effect, effectElement->renderer());
        effectElement->setStandardAttributes(effect.get());
        effect->setEffectBoundaries(SVGLengthContext::resolveRectangle<SVGFilterPrimitiveStandardAttributes>(effectElement, filterElement->primitiveUnits()->currentValue()->enumValue(), targetBoundingBox));
        effect->setOperatingColorSpace(
            effectElement->renderer()->style()->svgStyle().colorInterpolationFilters() == CI_LINEARRGB ? ColorSpaceLinearRGB : ColorSpaceDeviceRGB);
        builder->add(AtomicString(effectElement->result()->currentValue()->value()), effect);
    }
    return builder.release();
}

static void beginDeferredFilter(GraphicsContext* context, FilterData* filterData)
{
    // FIXME: Create a new GraphicsContext here to replace the existing context instead
    // of nesting recordings.
    context->beginRecording(filterData->boundaries);
}

static void endDeferredFilter(GraphicsContext* context, FilterData* filterData)
{
    // FIXME: maybe filterData should just hold onto SourceGraphic after creation?
    SourceGraphic* sourceGraphic = static_cast<SourceGraphic*>(filterData->builder->getEffectById(SourceGraphic::effectName()));
    ASSERT(sourceGraphic);
    sourceGraphic->setPicture(context->endRecording());
}

static void drawDeferredFilter(GraphicsContext* context, FilterData* filterData, SVGFilterElement* filterElement)
{
    SkiaImageFilterBuilder builder(context);
    SourceGraphic* sourceGraphic = static_cast<SourceGraphic*>(filterData->builder->getEffectById(SourceGraphic::effectName()));
    ASSERT(sourceGraphic);
    builder.setSourceGraphic(sourceGraphic);
    RefPtr<ImageFilter> imageFilter = builder.build(filterData->builder->lastEffect(), ColorSpaceDeviceRGB);
    FloatRect boundaries = filterData->boundaries;
    context->save();

    // Clip drawing of filtered image to the minimum required paint rect.
    FilterEffect* lastEffect = filterData->builder->lastEffect();
    context->clipRect(lastEffect->determineAbsolutePaintRect(lastEffect->maxEffectRect()));
    if (filterElement->hasAttribute(SVGNames::filterResAttr)) {
        // Get boundaries in device coords.
        // FIXME: See crbug.com/382491. Is the use of getCTM OK here, given it does not include device
        // zoom or High DPI adjustments?
        FloatSize size = context->getCTM().mapSize(boundaries.size());
        // Compute the scale amount required so that the resulting offscreen is exactly filterResX by filterResY pixels.
        float filterResScaleX = filterElement->filterResX()->currentValue()->value() / size.width();
        float filterResScaleY = filterElement->filterResY()->currentValue()->value() / size.height();
        // Scale the CTM so the primitive is drawn to filterRes.
        context->scale(filterResScaleX, filterResScaleY);
        // Create a resize filter with the inverse scale.
        AffineTransform resizeMatrix;
        resizeMatrix.scale(1 / filterResScaleX, 1 / filterResScaleY);
        imageFilter = builder.buildTransform(resizeMatrix, imageFilter.get());
    }

    // See crbug.com/382491.
    if (!RuntimeEnabledFeatures::slimmingPaintEnabled()) {
        // If the CTM contains rotation or shearing, apply the filter to
        // the unsheared/unrotated matrix, and do the shearing/rotation
        // as a final pass.
        AffineTransform ctm = context->getCTM();
        if (ctm.b() || ctm.c()) {
            AffineTransform scaleAndTranslate;
            scaleAndTranslate.translate(ctm.e(), ctm.f());
            scaleAndTranslate.scale(ctm.xScale(), ctm.yScale());
            ASSERT(scaleAndTranslate.isInvertible());
            AffineTransform shearAndRotate = scaleAndTranslate.inverse();
            shearAndRotate.multiply(ctm);
            context->setCTM(scaleAndTranslate);
            imageFilter = builder.buildTransform(shearAndRotate, imageFilter.get());
        }
    }
    context->beginLayer(1, CompositeSourceOver, &boundaries, ColorFilterNone, imageFilter.get());
    context->endLayer();
    context->restore();
}

bool RenderSVGResourceFilter::prepareEffect(RenderObject* object, GraphicsContext*& context)
{
    ASSERT(object);
    ASSERT(context);

    clearInvalidationMask();

    if (m_filter.contains(object)) {
        FilterData* filterData = m_filter.get(object);
        if (filterData->state == FilterData::PaintingSource)
            filterData->state = FilterData::CycleDetected;
        return false; // Already built, or we're in a cycle. Regardless, just do nothing more now.
    }

    OwnPtrWillBeRawPtr<FilterData> filterData = FilterData::create();
    FloatRect targetBoundingBox = object->objectBoundingBox();

    SVGFilterElement* filterElement = toSVGFilterElement(element());
    filterData->boundaries = SVGLengthContext::resolveRectangle<SVGFilterElement>(filterElement, filterElement->filterUnits()->currentValue()->enumValue(), targetBoundingBox);
    if (filterData->boundaries.isEmpty())
        return false;

    // Create the SVGFilter object.
    FloatRect drawingRegion = object->strokeBoundingBox();
    drawingRegion.intersect(filterData->boundaries);
    bool primitiveBoundingBoxMode = filterElement->primitiveUnits()->currentValue()->enumValue() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX;
    filterData->filter = SVGFilter::create(enclosingIntRect(drawingRegion), targetBoundingBox, filterData->boundaries, primitiveBoundingBoxMode);

    // Create all relevant filter primitives.
    filterData->builder = buildPrimitives(filterData->filter.get());
    if (!filterData->builder)
        return false;

    FilterEffect* lastEffect = filterData->builder->lastEffect();
    if (!lastEffect)
        return false;

    lastEffect->determineFilterPrimitiveSubregion(ClipToFilterRegion);

    FilterData* data = filterData.get();
    m_filter.set(object, filterData.release());
    beginDeferredFilter(context, data);
    return true;
}

void RenderSVGResourceFilter::finishEffect(RenderObject* object, GraphicsContext*& context)
{
    ASSERT(object);
    ASSERT(context);

    FilterData* filterData = m_filter.get(object);
    if (!filterData)
        return;

    switch (filterData->state) {
    case FilterData::CycleDetected:
        // applyResource detected a cycle. This can occur due to FeImage
        // referencing a source that makes use of the FEImage itself. This is
        // the first place we've hit the cycle, so set the state back to
        // PaintingSource so the return stack will continue correctly.
        filterData->state = FilterData::PaintingSource;
        return;

    case FilterData::PaintingSource:
        endDeferredFilter(context, filterData);
        break;

    case FilterData::Built: { } // Empty
    }

    drawDeferredFilter(context, filterData, toSVGFilterElement(element()));
    filterData->state = FilterData::Built;
}

FloatRect RenderSVGResourceFilter::resourceBoundingBox(const RenderObject* object)
{
    if (SVGFilterElement* element = toSVGFilterElement(this->element()))
        return SVGLengthContext::resolveRectangle<SVGFilterElement>(element, element->filterUnits()->currentValue()->enumValue(), object->objectBoundingBox());

    return FloatRect();
}

void RenderSVGResourceFilter::primitiveAttributeChanged(RenderObject* object, const QualifiedName& attribute)
{
    FilterMap::iterator it = m_filter.begin();
    FilterMap::iterator end = m_filter.end();
    SVGFilterPrimitiveStandardAttributes* primitve = static_cast<SVGFilterPrimitiveStandardAttributes*>(object->node());

    for (; it != end; ++it) {
        FilterData* filterData = it->value.get();
        if (filterData->state != FilterData::Built)
            continue;

        SVGFilterBuilder* builder = filterData->builder.get();
        FilterEffect* effect = builder->effectByRenderer(object);
        if (!effect)
            continue;
        // Since all effects shares the same attribute value, all
        // or none of them will be changed.
        if (!primitve->setFilterEffectAttribute(effect, attribute))
            return;
        builder->clearResultsRecursive(effect);

        // Issue paint invalidations for the image on the screen.
        markClientForInvalidation(it->key, PaintInvalidation);
    }
    markAllClientLayersForInvalidation();
}

}
