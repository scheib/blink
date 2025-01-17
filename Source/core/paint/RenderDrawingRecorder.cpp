// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/paint/RenderDrawingRecorder.h"

#include "core/rendering/RenderLayer.h"
#include "core/rendering/RenderObject.h"
#include "platform/RuntimeEnabledFeatures.h"

namespace blink {

RenderDrawingRecorder::RenderDrawingRecorder(GraphicsContext* context, const RenderObject& renderer, PaintPhase phase, const FloatRect& clip)
    : m_drawingRecorder(context, renderer.displayItemClient(), (DisplayItem::Type)phase, clip)
#ifndef NDEBUG
    , m_renderer(renderer)
#endif
{ }

RenderDrawingRecorder::~RenderDrawingRecorder()
{
    if (!RuntimeEnabledFeatures::slimmingPaintEnabled())
        return;

#ifndef NDEBUG
    m_drawingRecorder.setClientDebugString(String::format("renderer: \"%p %s\"", &m_renderer, m_renderer.debugName().utf8().data()));
#endif
}

} // namespace blink
