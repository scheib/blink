//
//  WebCoreFrame.m
//  WebCore
//
//  Created by Darin Adler on Fri Jul 12 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import "WebCoreFrame.h"

#import <render_frames.h>

@implementation WebCoreFrame

- (void)dealloc
{
    renderPart->deref();
    [super dealloc];
}

- (void)setRenderPart:(KHTMLRenderPart *)newPart;
{
    newPart->ref();
    if (renderPart) {
        renderPart->deref();
    }
    renderPart = newPart;
}

- (KHTMLRenderPart *)renderPart
{
    return renderPart;
}

@end
