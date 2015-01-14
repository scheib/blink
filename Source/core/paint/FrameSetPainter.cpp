// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/paint/FrameSetPainter.h"

#include "core/html/HTMLFrameSetElement.h"
#include "core/paint/GraphicsContextAnnotator.h"
#include "core/rendering/PaintInfo.h"
#include "core/rendering/RenderFrameSet.h"

namespace blink {

static Color borderStartEdgeColor()
{
    return Color(170, 170, 170);
}

static Color borderEndEdgeColor()
{
    return Color::black;
}

static Color borderFillColor()
{
    return Color(208, 208, 208);
}

void FrameSetPainter::paintColumnBorder(const PaintInfo& paintInfo, const IntRect& borderRect)
{
    if (!paintInfo.rect.intersects(borderRect))
        return;

    // FIXME: We should do something clever when borders from distinct framesets meet at a join.

    // Fill first.
    GraphicsContext* context = paintInfo.context;
    context->fillRect(borderRect, m_renderFrameSet.frameSet()->hasBorderColor() ? m_renderFrameSet.resolveColor(CSSPropertyBorderLeftColor) : borderFillColor());

    // Now stroke the edges but only if we have enough room to paint both edges with a little
    // bit of the fill color showing through.
    if (borderRect.width() >= 3) {
        context->fillRect(IntRect(borderRect.location(), IntSize(1, m_renderFrameSet.size().height())), borderStartEdgeColor());
        context->fillRect(IntRect(IntPoint(borderRect.maxX() - 1, borderRect.y()), IntSize(1, m_renderFrameSet.size().height())), borderEndEdgeColor());
    }
}

void FrameSetPainter::paintRowBorder(const PaintInfo& paintInfo, const IntRect& borderRect)
{
    if (!paintInfo.rect.intersects(borderRect))
        return;

    // FIXME: We should do something clever when borders from distinct framesets meet at a join.

    // Fill first.
    GraphicsContext* context = paintInfo.context;
    context->fillRect(borderRect, m_renderFrameSet.frameSet()->hasBorderColor() ? m_renderFrameSet.resolveColor(CSSPropertyBorderLeftColor) : borderFillColor());

    // Now stroke the edges but only if we have enough room to paint both edges with a little
    // bit of the fill color showing through.
    if (borderRect.height() >= 3) {
        context->fillRect(IntRect(borderRect.location(), IntSize(m_renderFrameSet.size().width(), 1)), borderStartEdgeColor());
        context->fillRect(IntRect(IntPoint(borderRect.x(), borderRect.maxY() - 1), IntSize(m_renderFrameSet.size().width(), 1)), borderEndEdgeColor());
    }
}

void FrameSetPainter::paint(const PaintInfo& paintInfo, const LayoutPoint& paintOffset)
{
    ANNOTATE_GRAPHICS_CONTEXT(paintInfo, &m_renderFrameSet);

    if (paintInfo.phase != PaintPhaseForeground)
        return;

    RenderObject* child = m_renderFrameSet.firstChild();
    if (!child)
        return;

    LayoutPoint adjustedPaintOffset = paintOffset + m_renderFrameSet.location();

    size_t rows = m_renderFrameSet.rows().m_sizes.size();
    size_t cols = m_renderFrameSet.columns().m_sizes.size();
    LayoutUnit borderThickness = m_renderFrameSet.frameSet()->border();

    LayoutUnit yPos = 0;
    for (size_t r = 0; r < rows; r++) {
        LayoutUnit xPos = 0;
        for (size_t c = 0; c < cols; c++) {
            child->paint(paintInfo, adjustedPaintOffset);
            xPos += m_renderFrameSet.columns().m_sizes[c];
            if (borderThickness && m_renderFrameSet.columns().m_allowBorder[c + 1]) {
                paintColumnBorder(paintInfo, pixelSnappedIntRect(
                    LayoutRect(adjustedPaintOffset.x() + xPos, adjustedPaintOffset.y() + yPos, borderThickness, m_renderFrameSet.size().height())));
                xPos += borderThickness;
            }
            child = child->nextSibling();
            if (!child)
                return;
        }
        yPos += m_renderFrameSet.rows().m_sizes[r];
        if (borderThickness && m_renderFrameSet.rows().m_allowBorder[r + 1]) {
            paintRowBorder(paintInfo, pixelSnappedIntRect(
                LayoutRect(adjustedPaintOffset.x(), adjustedPaintOffset.y() + yPos, m_renderFrameSet.size().width(), borderThickness)));
            yPos += borderThickness;
        }
    }
}

} // namespace blink
