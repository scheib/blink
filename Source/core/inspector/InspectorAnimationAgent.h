// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef InspectorAnimationAgent_h
#define InspectorAnimationAgent_h

#include "core/InspectorFrontend.h"
#include "core/css/CSSKeyframesRule.h"
#include "core/inspector/InspectorBaseAgent.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/text/WTFString.h"

namespace blink {

class AnimationNode;
class AnimationPlayer;
class Element;
class InspectorDOMAgent;
class TimingFunction;

class InspectorAnimationAgent final : public InspectorBaseAgent<InspectorAnimationAgent>, public InspectorBackendDispatcher::AnimationCommandHandler {
    WTF_MAKE_NONCOPYABLE(InspectorAnimationAgent);
public:
    static PassOwnPtrWillBeRawPtr<InspectorAnimationAgent> create(InspectorDOMAgent* domAgent)
    {
        return adoptPtrWillBeNoop(new InspectorAnimationAgent(domAgent));
    }

    // Base agent methods.
    virtual void setFrontend(InspectorFrontend*) override;
    virtual void clearFrontend() override;
    void reset();
    virtual void restore() override;

    // Protocol method implementations.
    virtual void getAnimationPlayersForNode(ErrorString*, int nodeId, bool includeSubtreeAnimations, RefPtr<TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer> >& animationPlayersArray) override;
    virtual void pauseAnimationPlayer(ErrorString*, const String& id, RefPtr<TypeBuilder::Animation::AnimationPlayer>&) override;
    virtual void playAnimationPlayer(ErrorString*, const String& id, RefPtr<TypeBuilder::Animation::AnimationPlayer>&) override;
    virtual void setAnimationPlayerCurrentTime(ErrorString*, const String& id, double currentTime, RefPtr<TypeBuilder::Animation::AnimationPlayer>&) override;
    virtual void getAnimationPlayerState(ErrorString*, const String& id, double* currentTime, bool* isRunning) override;
    virtual void startListening(ErrorString*, int nodeId, bool includeSubtreeAnimations) override;
    virtual void stopListening(ErrorString*) override;

    // API for InspectorInstrumentation
    void didCreateAnimationPlayer(AnimationPlayer&);

    // API for InspectorFrontend
    virtual void enable(ErrorString*) override;

    // Methods for other agents to use.
    AnimationPlayer* assertAnimationPlayer(ErrorString*, const String& id);

    virtual void trace(Visitor*) override;

private:
    InspectorAnimationAgent(InspectorDOMAgent*);

    PassRefPtr<TypeBuilder::Animation::AnimationNode> buildObjectForAnimationNode(AnimationNode*);
    PassRefPtr<TypeBuilder::Animation::AnimationPlayer> buildObjectForAnimationPlayer(AnimationPlayer&, PassRefPtr<TypeBuilder::Animation::KeyframesRule> keyframeRule = nullptr);
    PassRefPtr<TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer> > buildArrayForAnimationPlayers(Element&, const WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> >);

    RawPtrWillBeMember<InspectorDOMAgent> m_domAgent;
    InspectorFrontend::Animation* m_frontend;
    WillBeHeapHashMap<String, RefPtrWillBeMember<AnimationPlayer> > m_idToAnimationPlayer;

    RawPtrWillBeMember<Element> m_element;
    bool m_includeSubtree;
};

}

#endif // InspectorAnimationAgent_h
