#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "shared/ParameterSpecs.h"

//==============================================================================
namespace Parameters
{
    constexpr int versionHint = 1;

    inline const juce::ParameterID masterGainId { ParameterSpecs::masterGain.id, versionHint };

    inline const juce::ParameterID oscOctaveId { ParameterSpecs::oscOctave.id, versionHint };
    inline const juce::ParameterID osc1FineId { ParameterSpecs::osc1Fine.id, versionHint };
    inline const juce::ParameterID osc2FineId { ParameterSpecs::osc2Fine.id, versionHint };
    inline const juce::ParameterID oscMixId { ParameterSpecs::oscMix.id, versionHint };

    inline const juce::ParameterID envAttackId { ParameterSpecs::envAttack.id, versionHint };
    inline const juce::ParameterID envDecayId { ParameterSpecs::envDecay.id, versionHint };
    inline const juce::ParameterID envSustainId { ParameterSpecs::envSustain.id, versionHint };
    inline const juce::ParameterID envReleaseId { ParameterSpecs::envRelease.id, versionHint };
    inline const juce::ParameterID envAttackCurveId { ParameterSpecs::envAttackCurve.id, versionHint };
    inline const juce::ParameterID envDecayCurveId { ParameterSpecs::envDecayCurve.id, versionHint };
    inline const juce::ParameterID envReleaseCurveId { ParameterSpecs::envReleaseCurve.id, versionHint };

    inline const juce::ParameterID saturationDriveId { ParameterSpecs::saturationDrive.id, versionHint };
    inline const juce::ParameterID saturationDriveEnvId { ParameterSpecs::saturationDriveEnv.id, versionHint };

    inline const juce::ParameterID glideTimeId { ParameterSpecs::glideTime.id, versionHint };
    inline const juce::ParameterID glideModeId { ParameterSpecs::glideMode.id, versionHint };

    inline const juce::ParameterID pitchBendId { ParameterSpecs::pitchBend.id, versionHint };
    inline const juce::ParameterID pitchBendRangeId { ParameterSpecs::pitchBendRange.id, versionHint };

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
