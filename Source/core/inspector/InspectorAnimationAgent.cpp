// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "core/inspector/InspectorAnimationAgent.h"

#include "core/animation/Animation.h"
#include "core/animation/AnimationEffect.h"
#include "core/animation/AnimationNode.h"
#include "core/animation/AnimationPlayer.h"
#include "core/animation/ComputedTimingProperties.h"
#include "core/animation/ElementAnimation.h"
#include "core/animation/KeyframeEffectModel.h"
#include "core/animation/StringKeyframe.h"
#include "core/css/CSSKeyframeRule.h"
#include "core/css/CSSKeyframesRule.h"
#include "core/css/resolver/StyleResolver.h"
#include "core/inspector/InspectorDOMAgent.h"
#include "core/inspector/InspectorNodeIds.h"
#include "core/inspector/InspectorState.h"
#include "core/inspector/InspectorStyleSheet.h"
#include "platform/Decimal.h"
#include "platform/animation/TimingFunction.h"

namespace AnimationAgentState {
static const char animationAgentEnabled[] = "animationAgentEnabled";
}

namespace blink {

InspectorAnimationAgent::InspectorAnimationAgent(InspectorDOMAgent* domAgent)
    : InspectorBaseAgent<InspectorAnimationAgent>("Animation")
    , m_domAgent(domAgent)
    , m_frontend(nullptr)
    , m_element(nullptr)
{
}

void InspectorAnimationAgent::setFrontend(InspectorFrontend* frontend)
{
    m_frontend = frontend->animation();
}

void InspectorAnimationAgent::clearFrontend()
{
    m_instrumentingAgents->setInspectorAnimationAgent(nullptr);
    m_frontend = nullptr;
    reset();
}

void InspectorAnimationAgent::reset()
{
    m_idToAnimationPlayer.clear();
}

void InspectorAnimationAgent::restore()
{
    if (m_state->getBoolean(AnimationAgentState::animationAgentEnabled)) {
        ErrorString error;
        enable(&error);
    }
}

void InspectorAnimationAgent::enable(ErrorString*)
{
    m_state->setBoolean(AnimationAgentState::animationAgentEnabled, true);
    m_instrumentingAgents->setInspectorAnimationAgent(this);
}

PassRefPtr<TypeBuilder::Animation::AnimationNode> InspectorAnimationAgent::buildObjectForAnimationNode(AnimationNode* animationNode)
{
    ComputedTimingProperties computedTiming;
    animationNode->computedTiming(computedTiming);
    RefPtr<TypeBuilder::Animation::AnimationNode> animationObject = TypeBuilder::Animation::AnimationNode::create()
        .setDelay(computedTiming.delay())
        .setPlaybackRate(computedTiming.playbackRate())
        .setIterationStart(computedTiming.iterationStart())
        .setIterations(computedTiming.iterations())
        .setDuration(computedTiming.duration().getAsUnrestrictedDouble())
        .setDirection(computedTiming.direction())
        .setFill(computedTiming.fill())
        .setName(animationNode->name())
        .setBackendNodeId(InspectorNodeIds::idForNode(toAnimation(animationNode)->target()))
        .setEasing(animationNode->specifiedTiming().timingFunction->toString());
    return animationObject.release();
}

static String playerId(AnimationPlayer& player)
{
    return String::number(player.sequenceNumber());
}

PassRefPtr<TypeBuilder::Animation::AnimationPlayer> InspectorAnimationAgent::buildObjectForAnimationPlayer(AnimationPlayer& animationPlayer, PassRefPtr<TypeBuilder::Animation::KeyframesRule> keyframeRule)
{
    RefPtr<TypeBuilder::Animation::AnimationNode> animationObject = buildObjectForAnimationNode(animationPlayer.source());
    if (keyframeRule)
        animationObject->setKeyframesRule(keyframeRule);

    RefPtr<TypeBuilder::Animation::AnimationPlayer> playerObject = TypeBuilder::Animation::AnimationPlayer::create()
        .setId(playerId(animationPlayer))
        .setPausedState(animationPlayer.paused())
        .setPlayState(animationPlayer.playState())
        .setPlaybackRate(animationPlayer.playbackRate())
        .setStartTime(animationPlayer.startTime())
        .setCurrentTime(animationPlayer.currentTime())
        .setSource(animationObject.release());
    return playerObject.release();
}

static PassRefPtr<TypeBuilder::Animation::KeyframeStyle> buildObjectForStyleRuleKeyframe(StyleRuleKeyframe* keyframe, TimingFunction& easing)
{
    RefPtrWillBeRawPtr<InspectorStyle> inspectorStyle = InspectorStyle::create(InspectorCSSId(), keyframe->mutableProperties().ensureCSSStyleDeclaration(), 0);
    RefPtr<TypeBuilder::Animation::KeyframeStyle> keyframeObject = TypeBuilder::Animation::KeyframeStyle::create()
        .setOffset(keyframe->keyText())
        .setStyle(inspectorStyle->buildObjectForStyle())
        .setEasing(easing.toString());
    return keyframeObject.release();
}

static PassRefPtr<TypeBuilder::Animation::KeyframeStyle> buildObjectForStringKeyframe(const StringKeyframe* keyframe)
{
    RefPtrWillBeRawPtr<InspectorStyle> inspectorStyle = InspectorStyle::create(InspectorCSSId(), keyframe->propertySetForInspector().get()->ensureCSSStyleDeclaration(), 0);
    Decimal decimal = Decimal::fromDouble(keyframe->offset() * 100);
    String offset = decimal.toString();
    offset.append("%");

    RefPtr<TypeBuilder::Animation::KeyframeStyle> keyframeObject = TypeBuilder::Animation::KeyframeStyle::create()
        .setOffset(offset)
        .setStyle(inspectorStyle->buildObjectForStyle())
        .setEasing(keyframe->easing().toString());
    return keyframeObject.release();
}

static PassRefPtr<TypeBuilder::Animation::KeyframesRule> buildObjectForStyleRuleKeyframes(const AnimationPlayer& player, const StyleRuleKeyframes* keyframesRule)
{
    RefPtr<TypeBuilder::Array<TypeBuilder::Animation::KeyframeStyle> > keyframes = TypeBuilder::Array<TypeBuilder::Animation::KeyframeStyle>::create();
    const WillBeHeapVector<RefPtrWillBeMember<StyleRuleKeyframe> >& styleKeyframes = keyframesRule->keyframes();
    for (const auto& styleKeyframe : styleKeyframes) {
        WillBeHeapVector<RefPtrWillBeMember<Keyframe> > normalizedKeyframes = KeyframeEffectModelBase::normalizedKeyframesForInspector(toKeyframeEffectModelBase(toAnimation(player.source())->effect())->getFrames());
        TimingFunction* easing = nullptr;
        for (const auto& keyframe : normalizedKeyframes) {
            if (styleKeyframe->keys().contains(keyframe->offset()))
                easing = &keyframe->easing();
        }
        ASSERT(easing);
        keyframes->addItem(buildObjectForStyleRuleKeyframe(styleKeyframe.get(), *easing));
    }

    RefPtr<TypeBuilder::Animation::KeyframesRule> keyframesObject = TypeBuilder::Animation::KeyframesRule::create()
        .setKeyframes(keyframes);
    keyframesObject->setName(keyframesRule->name());
    return keyframesObject.release();
}

static PassRefPtr<TypeBuilder::Animation::KeyframesRule> buildObjectForAnimationKeyframes(const Animation* animation)
{
    if (!animation->effect()->isKeyframeEffectModel())
        return nullptr;
    const KeyframeEffectModelBase* effect = toKeyframeEffectModelBase(animation->effect());
    WillBeHeapVector<RefPtrWillBeMember<Keyframe> > normalizedKeyframes = KeyframeEffectModelBase::normalizedKeyframesForInspector(effect->getFrames());
    RefPtr<TypeBuilder::Array<TypeBuilder::Animation::KeyframeStyle> > keyframes = TypeBuilder::Array<TypeBuilder::Animation::KeyframeStyle>::create();

    for (const auto& keyframe : normalizedKeyframes) {
        // Ignore CSS Transitions
        if (!keyframe.get()->isStringKeyframe())
            continue;
        const StringKeyframe* stringKeyframe = toStringKeyframe(keyframe.get());
        keyframes->addItem(buildObjectForStringKeyframe(stringKeyframe));
    }
    RefPtr<TypeBuilder::Animation::KeyframesRule> keyframesObject = TypeBuilder::Animation::KeyframesRule::create()
        .setKeyframes(keyframes);
    return keyframesObject.release();
}

static PassRefPtr<TypeBuilder::Animation::KeyframesRule> buildObjectForKeyframesRule(const AnimationPlayer& player)
{
    const Element* element = toAnimation(player.source())->target();
    StyleResolver& styleResolver = element->ownerDocument()->ensureStyleResolver();
    CSSAnimations& cssAnimations = element->activeAnimations()->cssAnimations();
    const AtomicString animationName = cssAnimations.getAnimationNameForInspector(player);
    RefPtr<TypeBuilder::Animation::KeyframesRule> keyframeRule;

    if (!animationName.isNull()) {
        // CSS Animations
        const StyleRuleKeyframes* keyframes = styleResolver.findKeyframesRule(element, animationName);
        keyframeRule = buildObjectForStyleRuleKeyframes(player, keyframes);
    } else {
        // Web Animations
        keyframeRule = buildObjectForAnimationKeyframes(toAnimation(player.source()));
    }

    return keyframeRule;
}

PassRefPtr<TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer> > InspectorAnimationAgent::buildArrayForAnimationPlayers(Element& element, const WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> > players)
{
    RefPtr<TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer> > animationPlayersArray = TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer>::create();
    for (const auto& it : players) {
        AnimationPlayer& player = *(it.get());
        Animation* animation = toAnimation(player.source());
        if (!element.contains(animation->target()))
            continue;
        m_idToAnimationPlayer.set(playerId(player), &player);
        RefPtr<TypeBuilder::Animation::KeyframesRule> keyframeRule = buildObjectForKeyframesRule(player);
        animationPlayersArray->addItem(buildObjectForAnimationPlayer(player, keyframeRule));
    }
    return animationPlayersArray.release();
}

void InspectorAnimationAgent::getAnimationPlayersForNode(ErrorString* errorString, int nodeId, bool includeSubtreeAnimations, RefPtr<TypeBuilder::Array<TypeBuilder::Animation::AnimationPlayer> >& animationPlayersArray)
{
    Element* element = m_domAgent->assertElement(errorString, nodeId);
    if (!element)
        return;
    m_idToAnimationPlayer.clear();
    WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> > players;
    if (!includeSubtreeAnimations)
        players = ElementAnimation::getAnimationPlayers(*element);
    else
        players = element->ownerDocument()->timeline().getAnimationPlayers();
    animationPlayersArray = buildArrayForAnimationPlayers(*element, players);
}

void InspectorAnimationAgent::pauseAnimationPlayer(ErrorString* errorString, const String& id, RefPtr<TypeBuilder::Animation::AnimationPlayer>& animationPlayer)
{
    AnimationPlayer* player = assertAnimationPlayer(errorString, id);
    if (!player)
        return;
    if (player->playStateInternal() == AnimationPlayer::Idle)
        player->play();
    player->pause();
    animationPlayer = buildObjectForAnimationPlayer(*player);
}

void InspectorAnimationAgent::playAnimationPlayer(ErrorString* errorString, const String& id, RefPtr<TypeBuilder::Animation::AnimationPlayer>& animationPlayer)
{
    AnimationPlayer* player = assertAnimationPlayer(errorString, id);
    if (!player)
        return;
    player->play();
    animationPlayer = buildObjectForAnimationPlayer(*player);
}

void InspectorAnimationAgent::setAnimationPlayerCurrentTime(ErrorString* errorString, const String& id, double currentTime, RefPtr<TypeBuilder::Animation::AnimationPlayer>& animationPlayer)
{
    AnimationPlayer* player = assertAnimationPlayer(errorString, id);
    if (!player)
        return;
    player->setCurrentTime(currentTime);
    animationPlayer = buildObjectForAnimationPlayer(*player);
}

void InspectorAnimationAgent::getAnimationPlayerState(ErrorString* errorString, const String& id, double* currentTime, bool* isRunning)
{
    AnimationPlayer* player = assertAnimationPlayer(errorString, id);
    if (!player)
        return;
    *currentTime = player->currentTime();
    *isRunning = player->playing();
}

void InspectorAnimationAgent::startListening(ErrorString* errorString, int nodeId, bool includeSubtreeAnimations)
{
    Element* element = m_domAgent->assertElement(errorString, nodeId);
    if (!element)
        return;
    m_element = element;
    m_includeSubtree = includeSubtreeAnimations;
}

void InspectorAnimationAgent::stopListening(ErrorString*)
{
    m_element = nullptr;
}

void InspectorAnimationAgent::didCreateAnimationPlayer(AnimationPlayer& player)
{
    if (!m_element)
        return;
    Animation* animation = toAnimation(player.source());

    bool inSubtree = m_includeSubtree ? m_element->contains(animation->target()) : m_element->isSameNode(animation->target());
    if (!inSubtree)
        return;

    m_idToAnimationPlayer.set(playerId(player), &player);
    RefPtr<TypeBuilder::Animation::KeyframesRule> keyframeRule = buildObjectForKeyframesRule(player);
    m_frontend->animationPlayerCreated(buildObjectForAnimationPlayer(player, keyframeRule));
}

AnimationPlayer* InspectorAnimationAgent::assertAnimationPlayer(ErrorString* errorString, const String& id)
{
    AnimationPlayer* player = m_idToAnimationPlayer.get(id);
    if (!player) {
        *errorString = "Could not find animation player with given id";
        return nullptr;
    }
    return player;
}

void InspectorAnimationAgent::trace(Visitor* visitor)
{
#if ENABLE(OILPAN)
    visitor->trace(m_idToAnimationPlayer);
    visitor->trace(m_domAgent);
    visitor->trace(m_element);
#endif
    InspectorBaseAgent::trace(visitor);
}

}
