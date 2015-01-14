/**
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Google, Inc.  All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2012 Zoltan Herczeg <zherczeg@webkit.org>.
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

#ifndef SVGRenderingContext_h
#define SVGRenderingContext_h

#include "core/paint/FloatClipRecorder.h"
#include "core/paint/TransparencyRecorder.h"
#include "core/rendering/svg/RenderSVGResourceClipper.h"
#include "platform/graphics/paint/ClipPathRecorder.h"
#include "platform/transforms/AffineTransform.h"

namespace blink {

class RenderObject;
struct PaintInfo;
class RenderSVGResourceFilter;
class RenderSVGResourceMasker;

class SubtreeContentTransformScope {
public:
    SubtreeContentTransformScope(const AffineTransform&);
    ~SubtreeContentTransformScope();

private:
    AffineTransform m_savedContentTransformation;
};

// SVGRenderingContext
class SVGRenderingContext {
    STACK_ALLOCATED();
public:
    // Does not start rendering.
    SVGRenderingContext()
        : m_renderingFlags(0)
        , m_object(nullptr)
        , m_paintInfo(nullptr)
        , m_filter(nullptr)
        , m_clipper(nullptr)
        , m_clipperState(RenderSVGResourceClipper::ClipperNotApplied)
        , m_masker(nullptr)
    {
    }

    SVGRenderingContext(RenderObject* object, PaintInfo& paintinfo)
        : m_renderingFlags(0)
        , m_object(nullptr)
        , m_paintInfo(nullptr)
        , m_filter(nullptr)
        , m_clipper(nullptr)
        , m_clipperState(RenderSVGResourceClipper::ClipperNotApplied)
        , m_masker(nullptr)
    {
        prepareToRenderSVGContent(object, paintinfo);
    }

    // Automatically finishes context rendering.
    ~SVGRenderingContext();

    // Used by all SVG renderers who apply clip/filter/etc. resources to the renderer content.
    void prepareToRenderSVGContent(RenderObject*, PaintInfo&);
    bool isRenderingPrepared() const { return m_renderingFlags & RenderingPrepared; }
    bool isIsolationInstalled() const;

    static void renderSubtree(GraphicsContext*, RenderObject*);

    static float calculateScreenFontSizeScalingFactor(const RenderObject*);

private:
    // To properly revert partially successful initializtions in the destructor, we record all successful steps.
    enum RenderingFlags {
        RenderingPrepared = 1,
        PostApplyResources = 1 << 1,
        PrepareToRenderSVGContentWasCalled = 1 << 2
    };

    int m_renderingFlags;
    RawPtrWillBeMember<RenderObject> m_object;
    PaintInfo* m_paintInfo;
    IntRect m_savedPaintRect;
    RawPtrWillBeMember<RenderSVGResourceFilter> m_filter;
    RawPtrWillBeMember<RenderSVGResourceClipper> m_clipper;
    RenderSVGResourceClipper::ClipperState m_clipperState;
    RawPtrWillBeMember<RenderSVGResourceMasker> m_masker;
    OwnPtr<TransparencyRecorder> m_transparencyRecorder;
    OwnPtr<FloatClipRecorder> m_clipRecorder;
    OwnPtr<ClipPathRecorder> m_clipPathRecorder;
};

} // namespace blink

#endif // SVGRenderingContext_h
