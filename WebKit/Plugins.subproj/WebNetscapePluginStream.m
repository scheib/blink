/*
        WebNetscapePluginStream.h
	Copyright 2002, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebNetscapePluginStream.h>

#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebNetscapePluginEmbeddedView.h>
#import <WebKit/WebViewPrivate.h>

#import <Foundation/NSError_NSURLExtras.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLResponsePrivate.h>
#import <Foundation/NSURLRequest.h>

@interface WebNetscapePluginConnectionDelegate : WebBaseResourceHandleDelegate
{
    WebNetscapePluginStream *stream;
    WebBaseNetscapePluginView *view;
    NSMutableData *resourceData;
}
- initWithStream:(WebNetscapePluginStream *)theStream view:(WebBaseNetscapePluginView *)theView;
@end

@implementation WebNetscapePluginStream

- initWithRequest:(NSURLRequest *)theRequest
    pluginPointer:(NPP)thePluginPointer
       notifyData:(void *)theNotifyData
{
    [super init];

    if (!theRequest || !thePluginPointer || ![WebView _canHandleRequest:theRequest]) {
        [self release];
        return nil;
    }

    _startingRequest = [theRequest copy];

    [self setPluginPointer:thePluginPointer];

    WebBaseNetscapePluginView *view = (WebBaseNetscapePluginView *)instance->ndata;
    _loader = [[WebNetscapePluginConnectionDelegate alloc] initWithStream:self view:view]; 
    [_loader setDataSource:[view dataSource]];

    notifyData = theNotifyData;

    return self;
}

- (void)dealloc
{
    [_loader release];
    [_startingRequest release];
    [super dealloc];
}

- (void)start
{
    ASSERT(_startingRequest);
    [_loader loadWithRequest:_startingRequest];
    [_startingRequest release];
    _startingRequest = nil;
}

- (void)stop
{
    [_loader cancel];
}

@end

@implementation WebNetscapePluginConnectionDelegate

- initWithStream:(WebNetscapePluginStream *)theStream view:(WebBaseNetscapePluginView *)theView
{
    [super init];
    stream = [theStream retain];
    view = [theView retain];
    resourceData = [[NSMutableData alloc] init];
    return self;
}

- (void)releaseResources
{
    [stream release];
    stream = nil;
    [view release];
    view = nil;
    [resourceData release];
    resourceData = nil;
    [super releaseResources];
}

- (void)connection:(NSURLConnection *)con didReceiveResponse:(NSURLResponse *)theResponse
{
    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain]; 
    [stream setResponse:theResponse];
    [super connection:con didReceiveResponse:theResponse];
    if ([theResponse isKindOfClass:[NSHTTPURLResponse class]] &&
        [NSHTTPURLResponse isErrorStatusCode:[(NSHTTPURLResponse *)theResponse statusCode]]) {
        [stream receivedError:NPRES_NETWORK_ERR];
        NSError *error = [NSError _web_errorWithDomain:NSURLErrorDomain
                                                  code:NSURLErrorFileDoesNotExist
                                            failingURL:[[theResponse URL] absoluteString]];
        [self cancelWithError:error];
    }
    [self release];
}

- (void)connection:(NSURLConnection *)con didReceiveData:(NSData *)data
{
    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];
    if ([stream transferMode] == NP_ASFILE || [stream transferMode] == NP_ASFILEONLY) {
        [resourceData appendData:data];
    }

    [stream receivedData:data];
    [super connection:con didReceiveData:data];
    [self release];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)con
{
    [[view webView] _finishedLoadingResourceFromDataSource:[view dataSource]];
    [stream finishedLoadingWithData:resourceData];
    [super connectionDidFinishLoading:con];
}

- (void)connection:(NSURLConnection *)con didFailWithError:(NSError *)result
{
    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];
    [[view webView] _receivedError:result fromDataSource:[view dataSource]];
    [stream receivedError:NPRES_NETWORK_ERR];
    [super connection:con didFailWithError:result];
    [self release];
}

- (void)cancel
{
    // Since the plug-in is notified of the stream when the response is received,
    // only report an error if the response has been received.
    if ([self response]) {
        [stream receivedError:NPRES_USER_BREAK];
    }

    [super cancel];
}

@end
