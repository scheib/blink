/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
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

#import <KWQWindowWidget.h>
#import <Cocoa/Cocoa.h>

class KWQWindowWidgetPrivate
{
public:
    NSWindow *window;
};

static CFMutableDictionaryRef windowWidgets = NULL;

KWQWindowWidget *KWQWindowWidget::fromNSWindow(NSWindow *window)
{
    if (windowWidgets == NULL) {
	windowWidgets = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
    }
    
    KWQWindowWidget *widget = (KWQWindowWidget *)CFDictionaryGetValue(windowWidgets, window);
    if (widget == NULL) {
	widget = new KWQWindowWidget(window);
	CFDictionarySetValue(windowWidgets, window, widget);
    }

    return widget;
}


static void deleteOnWindowClose(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
    delete (KWQWindowWidget *)observer;
}

KWQWindowWidget::KWQWindowWidget(NSWindow *window) :
    d(new KWQWindowWidgetPrivate())
{
    d->window = [window retain];

    CFNotificationCenterAddObserver
	((CFNotificationCenterRef)[NSNotificationCenter defaultCenter],
	 this, deleteOnWindowClose, (CFStringRef)NSWindowWillCloseNotification, 
	 d->window, CFNotificationSuspensionBehaviorDeliverImmediately);
}

KWQWindowWidget::~KWQWindowWidget()
{
    CFDictionaryRemoveValue(windowWidgets, d->window);
    CFNotificationCenterRemoveObserver
	((CFNotificationCenterRef)[NSNotificationCenter defaultCenter],
	 this, (CFStringRef)NSWindowWillCloseNotification, d->window);
    [d->window release];
    delete d;
}

QSize KWQWindowWidget::sizeHint() const
{
    return size();
}

QSize KWQWindowWidget::minimumSizeHint() const
{
    return size();
}

QRect KWQWindowWidget::frameGeometry() const
{
    NSRect frame = [d->window frame];
    NSRect screenFrame = [[d->window screen] frame];
    return QRect((int)frame.origin.x, (int)(screenFrame.size.height - frame.origin.y - frame.size.height),
		 (int)frame.size.width, (int)frame.size.height);
}

QWidget *KWQWindowWidget::topLevelWidget() const
{
    return (QWidget *)this;
}

QPoint KWQWindowWidget::mapToGlobal(const QPoint &p) const
{
    NSRect screenFrame = [[d->window screen] frame];
    NSRect frame = [d->window frame];
    NSPoint windowPoint = NSMakePoint(p.x(), frame.size.height - p.y());

    NSPoint screenPoint = [d->window convertBaseToScreen:windowPoint];

    return QPoint((int)screenPoint.x, (int)(screenFrame.size.height - screenFrame.origin.y - screenPoint.y));
}

QPoint KWQWindowWidget::mapFromGlobal(const QPoint &p) const
{
    NSRect screenFrame = [[d->window screen] frame];
    NSRect frame = [d->window frame];
    NSPoint screenPoint = NSMakePoint(p.x(), screenFrame.size.height - screenFrame.origin.y - p.y());

    NSPoint windowPoint = [d->window convertScreenToBase:screenPoint];

    return QPoint((int)windowPoint.x, (int)(frame.size.height - windowPoint.y));
}

void KWQWindowWidget::setCursor(const QCursor &)
{
}

void KWQWindowWidget::internalSetGeometry( int x, int y, int w, int h )
{
    // FIXME: should try to avoid saving changes
    NSRect screenFrame = [[d->window screen] frame];
    [d->window setFrame:NSMakeRect(x, screenFrame.size.height - y - h,
			       w, h) display:NO];
}


