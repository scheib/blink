// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/streams/ReadableStream.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/ScriptFunction.h"
#include "bindings/core/v8/ScriptPromiseResolver.h"
#include "bindings/core/v8/V8Binding.h"
#include "core/dom/DOMException.h"
#include "core/dom/ExceptionCode.h"
#include "core/dom/ExecutionContext.h"
#include "core/streams/UnderlyingSource.h"

namespace blink {

namespace {

class ConstUndefined : public ScriptFunction {
public:
    static v8::Handle<v8::Function> create(ScriptState* scriptState)
    {
        return (new ConstUndefined(scriptState))->bindToV8Function();
    }

private:
    explicit ConstUndefined(ScriptState* scriptState) : ScriptFunction(scriptState) { }
    ScriptValue call(ScriptValue value) override
    {
        return ScriptValue(scriptState(), v8::Undefined(scriptState()->isolate()));
    }
};

} // namespace

ReadableStream::ReadableStream(ExecutionContext* executionContext, UnderlyingSource* source)
    : ActiveDOMObject(executionContext)
    , m_source(source)
    , m_isStarted(false)
    , m_isDraining(false)
    , m_isPulling(false)
    , m_state(Waiting)
    , m_ready(new WaitPromise(executionContext, this, WaitPromise::Ready))
    , m_closed(new ClosedPromise(executionContext, this, ClosedPromise::Closed))
{
    suspendIfNeeded();
}

ReadableStream::~ReadableStream()
{
}

String ReadableStream::stateString() const
{
    switch (m_state) {
    case Readable:
        return "readable";
    case Waiting:
        return "waiting";
    case Closed:
        return "closed";
    case Errored:
        return "errored";
    }
    ASSERT(false);
    return String();
}

bool ReadableStream::enqueuePreliminaryCheck()
{
    // This is a bit different from what spec says: it says we should throw
    // an exception here. But sometimes a caller is not in any JavaScript
    // context, and we don't want to throw an exception in such a case.
    if (m_state == Errored || m_state == Closed || m_isDraining)
        return false;

    return true;
}

bool ReadableStream::enqueuePostAction()
{
    m_isPulling = false;

    bool shouldApplyBackpressure = this->shouldApplyBackpressure();
    // this->shouldApplyBackpressure may call this->error().
    if (m_state == Errored)
        return false;

    if (m_state == Waiting) {
        // ReadableStream::hasPendingActivity return value gets false when
        // |m_state| is changed to Closed or Errored from Waiting or Readable.
        // On the other hand, the wrappers should be kept alive when |m_ready|
        // and |m_close| resolution and rejection are called. Hence we call
        // ScriptPromiseProperty::resolve and ScriptPromiseProperty::reject
        // *before* changing state, no matter if the state change actually
        // changes hasPendingActivity return value.
        m_ready->resolve(ToV8UndefinedGenerator());
        m_state = Readable;
    }

    return !shouldApplyBackpressure;
}

void ReadableStream::close()
{
    if (m_state == Waiting) {
        m_ready->resolve(ToV8UndefinedGenerator());
        m_closed->resolve(ToV8UndefinedGenerator());
        m_state = Closed;
    } else if (m_state == Readable) {
        m_isDraining = true;
    }
}

void ReadableStream::readPreliminaryCheck(ExceptionState& exceptionState)
{
    if (m_state == Waiting) {
        exceptionState.throwTypeError("read is called while state is waiting");
        return;
    }
    if (m_state == Closed) {
        exceptionState.throwTypeError("read is called while state is closed");
        return;
    }
    if (m_state == Errored) {
        exceptionState.throwDOMException(m_exception->code(), m_exception->message());
        return;
    }
}

void ReadableStream::readPostAction()
{
    ASSERT(m_state == Readable);
    if (isQueueEmpty()) {
        if (m_isDraining) {
            m_closed->resolve(ToV8UndefinedGenerator());
            m_state = Closed;
        } else {
            m_ready->reset();
            m_state = Waiting;
        }
    }
    callPullIfNeeded();
}

ScriptPromise ReadableStream::ready(ScriptState* scriptState)
{
    return m_ready->promise(scriptState->world());
}

ScriptPromise ReadableStream::cancel(ScriptState* scriptState, ScriptValue reason)
{
    if (m_state == Closed)
        return ScriptPromise::cast(scriptState, v8::Undefined(scriptState->isolate()));
    if (m_state == Errored)
        return ScriptPromise::rejectWithDOMException(scriptState, m_exception);

    ASSERT(m_state == Readable || m_state == Waiting);
    if (m_state == Waiting)
        m_ready->resolve(ToV8UndefinedGenerator());
    clearQueue();
    m_closed->resolve(ToV8UndefinedGenerator());
    m_state = Closed;
    return m_source->cancelSource(scriptState, reason).then(ConstUndefined::create(scriptState));
}

ScriptPromise ReadableStream::closed(ScriptState* scriptState)
{
    return m_closed->promise(scriptState->world());
}

void ReadableStream::error(PassRefPtrWillBeRawPtr<DOMException> exception)
{
    switch (m_state) {
    case Waiting:
        m_exception = exception;
        m_ready->reject(m_exception);
        m_closed->reject(m_exception);
        m_state = Errored;
        break;
    case Readable:
        clearQueue();
        m_exception = exception;
        m_ready->reset();
        m_ready->reject(m_exception);
        m_closed->reject(m_exception);
        m_state = Errored;
        break;
    default:
        break;
    }
}

void ReadableStream::didSourceStart()
{
    m_isStarted = true;
    callPullIfNeeded();
}

void ReadableStream::callPullIfNeeded()
{
    if (m_isPulling || m_isDraining || !m_isStarted || m_state == Closed || m_state == Errored)
        return;

    bool shouldApplyBackpressure = this->shouldApplyBackpressure();
    // this->shouldApplyBackpressure may call this->error().
    if (shouldApplyBackpressure || m_state == Errored)
        return;
    m_isPulling = true;
    m_source->pullSource();
}

bool ReadableStream::hasPendingActivity() const
{
    return m_state == Waiting || m_state == Readable;
}

void ReadableStream::trace(Visitor* visitor)
{
    visitor->trace(m_source);
    visitor->trace(m_ready);
    visitor->trace(m_closed);
    visitor->trace(m_exception);
    ActiveDOMObject::trace(visitor);
}

} // namespace blink
