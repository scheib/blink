// Copyright 2015 The Chromium Authors. All rights reserved.
//
// The Chromium Authors can be found at
// http://src.chromium.org/svn/trunk/src/AUTHORS
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GraphicsContextClient_h
#define GraphicsContextClient_h

#include "platform/PlatformExport.h"

struct SkRect;
class SkPaint;

namespace blink {

class GraphicsContext;

class PLATFORM_EXPORT GraphicsContextClient {
public:
    enum DrawType {
        UntransformedUnclippedFill,
        Fill,
        FillOrStroke
    };

    enum ImageType {
        NoImage,
        OpaqueImage,
        NonOpaqueImage
    };

    void willDrawRect(GraphicsContext*, const SkRect&, const SkPaint*, ImageType, DrawType);
    virtual ~GraphicsContextClient() { }
protected:

    // Called when the draw about to be executed will overwrite the entire canvas.
    virtual void willOverwriteCanvas() = 0;
};

} // blink

#endif
