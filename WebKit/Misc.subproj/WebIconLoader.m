//
//  WebIconLoader.m
//  WebKit
//
//  Created by Chris Blumenberg on Thu Jul 18 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <WebKit/WebIconLoader.h>

#import <WebFoundation/WebFoundation.h>
#import <WebFoundation/WebNSURLExtras.h>

@interface WebIconLoaderPrivate : NSObject
{
@public
    WebResourceHandle *resourceHandle;
    id delegate;
    NSURL *url;
}

@end;

@implementation WebIconLoaderPrivate

- (void)dealloc
{
    [url release];
    [resourceHandle release];
    [super dealloc];
}

@end;

@implementation WebIconLoader

+ (NSImage *)defaultIcon
{
    return [NSImage imageNamed:@"url_icon"];
}

- initWithURL:(NSURL *)iconURL
{
    [super init];
    _private = [[WebIconLoaderPrivate alloc] init];
    _private->url = [iconURL retain];
    return self;
}

- (void)dealloc
{
    [_private release];
    [super dealloc];
}

- (void)setDelegate:(id)delegate
{
    _private->delegate = delegate;
}

- (void)startLoading
{
    if([_private->url isFileURL]){
        NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
        [_private->delegate receivedPageIcon:[workspace iconForFile:[_private->url path]]];
    }else{
        _private->resourceHandle = [[WebResourceHandle alloc] initWithURL:_private->url];
        [_private->resourceHandle addClient:self];
        [_private->resourceHandle loadInBackground];
    }
}

- (void)startLoadingOnlyFromCache
{
    [self startLoading];
}

- (void)stopLoading
{
    [_private->resourceHandle cancelLoadInBackground];
}

- (void)WebResourceHandleDidBeginLoading:(WebResourceHandle *)sender
{

}

- (void)WebResourceHandleDidCancelLoading:(WebResourceHandle *)sender
{

}

- (void)WebResourceHandleDidFinishLoading:(WebResourceHandle *)sender data:(NSData *)data
{
    NSImage *image = [[NSImage alloc] initWithData:data];
    if(image){
        [_private->delegate receivedPageIcon:[image autorelease]];
    }
}


- (void)WebResourceHandle:(WebResourceHandle *)sender resourceDataDidBecomeAvailable:(NSData *)data
{

}

- (void)WebResourceHandle:(WebResourceHandle *)sender resourceDidFailLoadingWithResult:(WebError *)result
{

}

- (void)WebResourceHandle:(WebResourceHandle *)sender didRedirectToURL:(NSURL *)url
{

}

@end
