#pragma once

#include "ParameterSpecs.h"
#include "../dsp/SubSynth.h"

//==============================================================================
template <typename ValueOf>
SubSynth::Settings makeSynthSettings (ValueOf&& valueOf)
{
    using namespace ParameterSpecs;

    SubSynth::Settings settings;
    auto& voice = settings.voice;
    voice.octave = static_cast<int> (valueOf (oscOctave));
    voice.osc1Fine = valueOf (osc1Fine);
    voice.osc2Fine = valueOf (osc2Fine);
    voice.oscMix = valueOf (oscMix);
    voice.envelope.attack = valueOf (envAttack);
    voice.envelope.decay = valueOf (envDecay);
    voice.envelope.sustain = valueOf (envSustain);
    voice.envelope.release = valueOf (envRelease);
    voice.envelope.attackCurve = valueOf (envAttackCurve);
    voice.envelope.decayCurve = valueOf (envDecayCurve);
    voice.envelope.releaseCurve = valueOf (envReleaseCurve);
    voice.saturationDrive = valueOf (saturationDrive);
    voice.saturationDriveEnv = valueOf (saturationDriveEnv);
    voice.glideTime = valueOf (glideTime);
    voice.glideMode = valueOf (glideMode) >= 0.5f ? GlideMode::fixedRate : GlideMode::fixedTime;
    voice.pitchBendRange = valueOf (pitchBendRange);
    settings.masterGainDb = valueOf (masterGain);
    return settings;
}
