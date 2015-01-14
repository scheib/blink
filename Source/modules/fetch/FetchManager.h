// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FetchManager_h
#define FetchManager_h

#include "bindings/core/v8/ScriptPromise.h"
#include "wtf/HashSet.h"
#include "wtf/OwnPtr.h"

namespace blink {

class ExecutionContext;
class FetchRequestData;
class ScriptState;

class FetchManager final : public NoBaseWillBeGarbageCollectedFinalized<FetchManager> {
public:
    static PassOwnPtrWillBeRawPtr<FetchManager> create(ExecutionContext* executionContext)
    {
        return adoptPtrWillBeNoop(new FetchManager(executionContext));
    }
    ~FetchManager();
    ScriptPromise fetch(ScriptState*, const FetchRequestData*);
    void stop();
    bool isStopped() const { return m_isStopped; }

    void trace(Visitor*);

private:
    class Loader;

    explicit FetchManager(ExecutionContext*);
    // Removes loader from |m_loaders|.
    void onLoaderFinished(Loader*);

    RawPtrWillBeMember<ExecutionContext> m_executionContext;
    HashSet<OwnPtr<Loader> > m_loaders;
    bool m_isStopped;
};

} // namespace blink

#endif // FetchManager_h
