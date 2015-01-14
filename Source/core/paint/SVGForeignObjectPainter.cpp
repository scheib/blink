// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/paint/SVGForeignObjectPainter.h"

#include "core/paint/BlockPainter.h"
#include "core/paint/FloatClipRecorder.h"
#include "core/paint/TransformRecorder.h"
#include "core/rendering/PaintInfo.h"
#include "core/rendering/svg/RenderSVGForeignObject.h"
#include "core/rendering/svg/SVGRenderSupport.h"
#include "core/rendering/svg/SVGRenderingContext.h"
#include "platform/graphics/GraphicsContextStateSaver.h"

namespace blink {

void SVGForeignObjectPainter::paint(const PaintInfo& paintInfo)
{
    if (paintInfo.phase != PaintPhaseForeground && paintInfo.phase != PaintPhaseSelection)
        return;

    PaintInfo childPaintInfo(paintInfo);
    GraphicsContextStateSaver stateSaver(*childPaintInfo.context);
    TransformRecorder transformRecorder(*childPaintInfo.context, m_renderSVGForeignObject.displayItemClient(), m_renderSVGForeignObject.localTransform());

    // When transitioning from SVG to block painters we need to keep the PaintInfo rect up-to-date
    // because it can be used for clipping.
    m_renderSVGForeignObject.updatePaintInfoRect(childPaintInfo.rect);

    OwnPtr<FloatClipRecorder> clipRecorder;
    if (SVGRenderSupport::isOverflowHidden(&m_renderSVGForeignObject))
        clipRecorder = adoptPtr(new FloatClipRecorder(*childPaintInfo.context, m_renderSVGForeignObject.displayItemClient(), childPaintInfo.phase, m_renderSVGForeignObject.viewportRect()));

    SVGRenderingContext renderingContext;
    bool continueRendering = true;
    if (paintInfo.phase == PaintPhaseForeground) {
        renderingContext.prepareToRenderSVGContent(&m_renderSVGForeignObject, childPaintInfo);
        continueRendering = renderingContext.isRenderingPrepared();
    }

    if (continueRendering) {
        // Paint all phases of FO elements atomically as though the FO element established its own stacking context.
        bool preservePhase = paintInfo.phase == PaintPhaseSelection || paintInfo.phase == PaintPhaseTextClip;
        const LayoutPoint childPoint = IntPoint();
        childPaintInfo.phase = preservePhase ? paintInfo.phase : PaintPhaseBlockBackground;
        BlockPainter(m_renderSVGForeignObject).paint(childPaintInfo, childPoint);
        if (!preservePhase) {
            childPaintInfo.phase = PaintPhaseChildBlockBackgrounds;
            BlockPainter(m_renderSVGForeignObject).paint(childPaintInfo, childPoint);
            childPaintInfo.phase = PaintPhaseFloat;
            BlockPainter(m_renderSVGForeignObject).paint(childPaintInfo, childPoint);
            childPaintInfo.phase = PaintPhaseForeground;
            BlockPainter(m_renderSVGForeignObject).paint(childPaintInfo, childPoint);
            childPaintInfo.phase = PaintPhaseOutline;
            BlockPainter(m_renderSVGForeignObject).paint(childPaintInfo, childPoint);
        }
    }
}

} // namespace blink
