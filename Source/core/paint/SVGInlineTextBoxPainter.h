// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SVGInlineTextBoxPainter_h
#define SVGInlineTextBoxPainter_h

#include "core/rendering/style/RenderStyleConstants.h"
#include "core/rendering/svg/RenderSVGResourcePaintServer.h"

namespace blink {

class FloatPoint;
class Font;
class GraphicsContext;
struct PaintInfo;
class LayoutPoint;
class RenderStyle;
class SVGInlineTextBox;
struct SVGTextFragment;
class TextRun;
class DocumentMarker;

class SVGInlineTextBoxPainter {
public:
    SVGInlineTextBoxPainter(SVGInlineTextBox& svgInlineTextBox) : m_svgInlineTextBox(svgInlineTextBox) { }
    void paint(const PaintInfo&, const LayoutPoint&);
    void paintSelectionBackground(const PaintInfo&);
    virtual void paintTextMatchMarker(GraphicsContext*, const FloatPoint&, DocumentMarker*, RenderStyle*, const Font&);

private:
    bool shouldPaintSelection() const;
    void paintTextFragments(const PaintInfo&, RenderObject&);
    void paintDecoration(const PaintInfo&, TextDecoration, const SVGTextFragment&);
    void paintTextWithShadows(const PaintInfo&, RenderStyle*, TextRun&, const SVGTextFragment&, int startPosition, int endPosition, RenderSVGResourceMode);
    void paintText(const PaintInfo&, RenderStyle*, RenderStyle* selectionStyle, const SVGTextFragment&, RenderSVGResourceMode, bool shouldPaintSelection);

    SVGInlineTextBox& m_svgInlineTextBox;
};

} // namespace blink

#endif // SVGInlineTextBoxPainter_h
