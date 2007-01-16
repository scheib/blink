/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "ChromeClientQt.h"

#include "Frame.h"
#include "FrameView.h"
#include "FrameLoadRequest.h"

#include "qwebpage.h"

#define notImplemented() do { fprintf(stderr, "FIXME: UNIMPLEMENTED: %s:%d\n", __FILE__, __LINE__); } while(0)

namespace WebCore
{

    
ChromeClientQt::ChromeClientQt(QWebPage* webPage)
    : m_webPage(webPage)
{
    
}


ChromeClientQt::~ChromeClientQt()
{
    
}

void ChromeClientQt::ref()
{
    Shared<ChromeClientQt>::ref();
}

void ChromeClientQt::deref()
{
    Shared<ChromeClientQt>::deref();
}

void ChromeClientQt::setWindowRect(const FloatRect& rect)
{
    if (!m_webPage)
        return;
//     m_webPage->emit setWindowRect(QRect(qRound(r.x()), qRound(r.y()),
//                                         qRound(r.width()), qRound(r.height())));
}


FloatRect ChromeClientQt::windowRect()
{
    if (!m_webPage)
        return FloatRect();

    return IntRect(m_webPage->topLevelWidget()->geometry());
}


FloatRect ChromeClientQt::pageRect()
{
    if (!m_webPage)
        return FloatRect();
    return FloatRect(QRectF(m_webPage->rect()));
}


float ChromeClientQt::scaleFactor()
{
    notImplemented();
    return 1;
}


void ChromeClientQt::focus()
{
    if (!m_webPage)
        return;
    m_webPage->setFocus();
}


void ChromeClientQt::unfocus()
{
    if (!m_webPage)
        return;
    m_webPage->clearFocus();
}

bool ChromeClientQt::canTakeFocus(FocusDirection)
{
    if (!m_webPage)
        return false;
    return m_webPage->focusPolicy() != Qt::NoFocus;
}

void ChromeClientQt::takeFocus(FocusDirection)
{
    if (!m_webPage)
        return;
    m_webPage->clearFocus();
}


Page* ChromeClientQt::createWindow(const FrameLoadRequest& request)
{
    //QWebPage *newPage = m_webPage->createWindow(...);
    notImplemented();
    return 0;
}


Page* ChromeClientQt::createModalDialog(const FrameLoadRequest&)
{
    notImplemented();
    return 0;
}


void ChromeClientQt::show()
{
    if (!m_webPage)
        return;
    m_webPage->topLevelWidget()->show();
}


bool ChromeClientQt::canRunModal()
{
    notImplemented();
    return false;
}


void ChromeClientQt::runModal()
{
    notImplemented();
}


void ChromeClientQt::setToolbarsVisible(bool)
{
    notImplemented();
}


bool ChromeClientQt::toolbarsVisible()
{
    notImplemented();
    return false;
}


void ChromeClientQt::setStatusbarVisible(bool)
{
    notImplemented();
}


bool ChromeClientQt::statusbarVisible()
{
    notImplemented();
    return false;
}


void ChromeClientQt::setScrollbarsVisible(bool)
{
    notImplemented();
}


bool ChromeClientQt::scrollbarsVisible()
{
    notImplemented();
    return true;
}


void ChromeClientQt::setMenubarVisible(bool)
{
    notImplemented();
}

bool ChromeClientQt::menubarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientQt::setResizable(bool)
{
    notImplemented();
}

void ChromeClientQt::addMessageToConsole(const String& message, unsigned int lineNumber,
                                         const String& sourceID)
{
    notImplemented();
}

void ChromeClientQt::chromeDestroyed()
{
    notImplemented();
}

bool ChromeClientQt::canRunBeforeUnloadConfirmPanel()
{
    notImplemented();
}

bool ChromeClientQt::runBeforeUnloadConfirmPanel(const String& message, Frame* frame)
{
    notImplemented();
}

void ChromeClientQt::closeWindowSoon()
{
    notImplemented();
}

}


