/*
 * Copyright (C) 2004, 2005, 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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
#include "core/svg/SVGLengthContext.h"

#include "bindings/core/v8/ExceptionMessages.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/SVGNames.h"
#include "core/css/CSSHelper.h"
#include "core/dom/ExceptionCode.h"
#include "core/rendering/RenderView.h"
#include "core/rendering/svg/RenderSVGRoot.h"
#include "core/rendering/svg/RenderSVGViewportContainer.h"
#include "core/svg/SVGSVGElement.h"
#include "platform/fonts/FontMetrics.h"

namespace blink {

SVGLengthContext::SVGLengthContext(const SVGElement* context)
    : m_context(context)
{
}

FloatRect SVGLengthContext::resolveRectangle(const SVGElement* context, SVGUnitTypes::SVGUnitType type, const FloatRect& viewport, PassRefPtrWillBeRawPtr<SVGLength> passX, PassRefPtrWillBeRawPtr<SVGLength> passY, PassRefPtrWillBeRawPtr<SVGLength> passWidth, PassRefPtrWillBeRawPtr<SVGLength> passHeight)
{
    RefPtrWillBeRawPtr<SVGLength> x = passX;
    RefPtrWillBeRawPtr<SVGLength> y = passY;
    RefPtrWillBeRawPtr<SVGLength> width = passWidth;
    RefPtrWillBeRawPtr<SVGLength> height = passHeight;

    ASSERT(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type != SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE && !viewport.isEmpty()) {
        const FloatSize& viewportSize = viewport.size();
        return FloatRect(
            convertValueFromPercentageToUserUnits(*x, viewportSize) + viewport.x(),
            convertValueFromPercentageToUserUnits(*y, viewportSize) + viewport.y(),
            convertValueFromPercentageToUserUnits(*width, viewportSize),
            convertValueFromPercentageToUserUnits(*height, viewportSize));
    }

    SVGLengthContext lengthContext(context);
    return FloatRect(x->value(lengthContext), y->value(lengthContext), width->value(lengthContext), height->value(lengthContext));
}

FloatPoint SVGLengthContext::resolvePoint(const SVGElement* context, SVGUnitTypes::SVGUnitType type, PassRefPtrWillBeRawPtr<SVGLength> passX, PassRefPtrWillBeRawPtr<SVGLength> passY)
{
    RefPtrWillBeRawPtr<SVGLength> x = passX;
    RefPtrWillBeRawPtr<SVGLength> y = passY;

    ASSERT(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) {
        SVGLengthContext lengthContext(context);
        return FloatPoint(x->value(lengthContext), y->value(lengthContext));
    }

    // FIXME: valueAsPercentage() won't be correct for eg. cm units. They need to be resolved in user space and then be considered in objectBoundingBox space.
    return FloatPoint(x->valueAsPercentage(), y->valueAsPercentage());
}

float SVGLengthContext::resolveLength(const SVGElement* context, SVGUnitTypes::SVGUnitType type, PassRefPtrWillBeRawPtr<SVGLength> passX)
{
    RefPtrWillBeRawPtr<SVGLength> x = passX;

    ASSERT(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) {
        SVGLengthContext lengthContext(context);
        return x->value(lengthContext);
    }

    // FIXME: valueAsPercentage() won't be correct for eg. cm units. They need to be resolved in user space and then be considered in objectBoundingBox space.
    return x->valueAsPercentage();
}

float SVGLengthContext::convertValueToUserUnits(float value, SVGLengthMode mode, SVGLengthType fromUnit, ExceptionState& exceptionState) const
{
    switch (fromUnit) {
    case LengthTypeUnknown:
        exceptionState.throwDOMException(NotSupportedError, ExceptionMessages::argumentNullOrIncorrectType(3, "SVGLengthType"));
        return 0;
    case LengthTypeNumber:
        return value;
    case LengthTypePX:
        return value;
    case LengthTypePercentage:
        return convertValueFromPercentageToUserUnits(value, mode, exceptionState) / 100;
    case LengthTypeEMS:
        return convertValueFromEMSToUserUnits(value, exceptionState);
    case LengthTypeEXS:
        return convertValueFromEXSToUserUnits(value, exceptionState);
    case LengthTypeCM:
        return value * cssPixelsPerCentimeter;
    case LengthTypeMM:
        return value * cssPixelsPerMillimeter;
    case LengthTypeIN:
        return value * cssPixelsPerInch;
    case LengthTypePT:
        return value * cssPixelsPerPoint;
    case LengthTypePC:
        return value * cssPixelsPerPica;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

float SVGLengthContext::convertValueFromUserUnits(float value, SVGLengthMode mode, SVGLengthType toUnit, ExceptionState& exceptionState) const
{
    switch (toUnit) {
    case LengthTypeUnknown:
        exceptionState.throwDOMException(NotSupportedError, ExceptionMessages::argumentNullOrIncorrectType(3, "SVGLengthType"));
        return 0;
    case LengthTypeNumber:
        return value;
    case LengthTypePercentage:
        return convertValueFromUserUnitsToPercentage(value * 100, mode, exceptionState);
    case LengthTypeEMS:
        return convertValueFromUserUnitsToEMS(value, exceptionState);
    case LengthTypeEXS:
        return convertValueFromUserUnitsToEXS(value, exceptionState);
    case LengthTypePX:
        return value;
    case LengthTypeCM:
        return value / cssPixelsPerCentimeter;
    case LengthTypeMM:
        return value / cssPixelsPerMillimeter;
    case LengthTypeIN:
        return value / cssPixelsPerInch;
    case LengthTypePT:
        return value / cssPixelsPerPoint;
    case LengthTypePC:
        return value / cssPixelsPerPica;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

float SVGLengthContext::convertValueFromUserUnitsToPercentage(float value, SVGLengthMode mode, ExceptionState& exceptionState) const
{
    FloatSize viewportSize;
    if (!determineViewport(viewportSize)) {
        exceptionState.throwDOMException(NotSupportedError, "The viewport could not be determined.");
        return 0;
    }

    switch (mode) {
    case LengthModeWidth:
        return value / viewportSize.width() * 100;
    case LengthModeHeight:
        return value / viewportSize.height() * 100;
    case LengthModeOther:
        return value / sqrtf(viewportSize.diagonalLengthSquared() / 2) * 100;
    };

    ASSERT_NOT_REACHED();
    return 0;
}

float SVGLengthContext::convertValueFromPercentageToUserUnits(float value, SVGLengthMode mode, ExceptionState& exceptionState) const
{
    FloatSize viewportSize;
    if (!determineViewport(viewportSize)) {
        exceptionState.throwDOMException(NotSupportedError, "The viewport could not be determined.");
        return 0;
    }
    return convertValueFromPercentageToUserUnits(value, mode, viewportSize);
}

float SVGLengthContext::convertValueFromPercentageToUserUnits(float value, SVGLengthMode mode, const FloatSize& viewportSize)
{
    switch (mode) {
    case LengthModeWidth:
        return value * viewportSize.width();
    case LengthModeHeight:
        return value * viewportSize.height();
    case LengthModeOther:
        return value * sqrtf(viewportSize.diagonalLengthSquared() / 2);
    }

    ASSERT_NOT_REACHED();
    return 0;
}

float SVGLengthContext::convertValueFromPercentageToUserUnits(const SVGLength& value, const FloatSize& viewportSize)
{
    switch (value.unitMode()) {
    case LengthModeWidth:
        return value.scaleByPercentage(viewportSize.width());
    case LengthModeHeight:
        return value.scaleByPercentage(viewportSize.height());
    case LengthModeOther:
        return value.scaleByPercentage(sqrtf(viewportSize.diagonalLengthSquared() / 2));
    }

    ASSERT_NOT_REACHED();
    return 0;
}

static inline RenderStyle* renderStyleForLengthResolving(const SVGElement* context)
{
    if (!context)
        return 0;

    const ContainerNode* currentContext = context;
    do {
        if (currentContext->renderer())
            return currentContext->renderer()->style();
        currentContext = currentContext->parentNode();
    } while (currentContext);

    // There must be at least a RenderSVGRoot renderer, carrying a style.
    ASSERT_NOT_REACHED();
    return 0;
}

float SVGLengthContext::convertValueFromUserUnitsToEMS(float value, ExceptionState& exceptionState) const
{
    RenderStyle* style = renderStyleForLengthResolving(m_context);
    if (!style) {
        exceptionState.throwDOMException(NotSupportedError, "No context could be found.");
        return 0;
    }

    float fontSize = style->specifiedFontSize();
    if (!fontSize) {
        exceptionState.throwDOMException(NotSupportedError, "No font-size could be determined.");
        return 0;
    }

    return value / fontSize;
}

float SVGLengthContext::convertValueFromEMSToUserUnits(float value, ExceptionState& exceptionState) const
{
    RenderStyle* style = renderStyleForLengthResolving(m_context);
    if (!style) {
        exceptionState.throwDOMException(NotSupportedError, "No context could be found.");
        return 0;
    }

    return value * style->specifiedFontSize();
}

float SVGLengthContext::convertValueFromUserUnitsToEXS(float value, ExceptionState& exceptionState) const
{
    RenderStyle* style = renderStyleForLengthResolving(m_context);
    if (!style) {
        exceptionState.throwDOMException(NotSupportedError, "No context could be found.");
        return 0;
    }

    // Use of ceil allows a pixel match to the W3Cs expected output of coords-units-03-b.svg
    // if this causes problems in real world cases maybe it would be best to remove this
    float xHeight = ceilf(style->fontMetrics().xHeight());
    if (!xHeight) {
        exceptionState.throwDOMException(NotSupportedError, "No x-height could be determined.");
        return 0;
    }

    return value / xHeight;
}

float SVGLengthContext::convertValueFromEXSToUserUnits(float value, ExceptionState& exceptionState) const
{
    RenderStyle* style = renderStyleForLengthResolving(m_context);
    if (!style) {
        exceptionState.throwDOMException(NotSupportedError, "No context could be found.");
        return 0;
    }

    // Use of ceil allows a pixel match to the W3Cs expected output of coords-units-03-b.svg
    // if this causes problems in real world cases maybe it would be best to remove this
    return value * ceilf(style->fontMetrics().xHeight());
}

bool SVGLengthContext::determineViewport(FloatSize& viewportSize) const
{
    if (!m_context)
        return false;

    // Root <svg> element lengths are resolved against the top level viewport.
    if (m_context->isOutermostSVGSVGElement()) {
        viewportSize = toSVGSVGElement(m_context)->currentViewportSize();
        return true;
    }

    // Take size from nearest viewport element.
    SVGElement* viewportElement = m_context->viewportElement();
    if (!isSVGSVGElement(viewportElement))
        return false;

    const SVGSVGElement& svg = toSVGSVGElement(*viewportElement);
    viewportSize = svg.currentViewBoxRect().size();
    if (viewportSize.isEmpty())
        viewportSize = svg.currentViewportSize();

    return true;
}

}
