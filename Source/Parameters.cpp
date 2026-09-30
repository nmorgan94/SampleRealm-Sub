#include "Parameters.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createLayout()
{
    return {
        std::make_unique<juce::AudioParameterFloat>(
            masterGainId,
            "Master Gain",
            juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f, 1.0f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("dB")),

        std::make_unique<juce::AudioParameterInt>(
            oscOctaveId, "Octave", -4, 0, -2,
            juce::AudioParameterIntAttributes().withLabel ("oct")),
        std::make_unique<juce::AudioParameterFloat>(
            osc1FineId, "Osc 1 Fine",
            juce::NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("ct")),

        std::make_unique<juce::AudioParameterFloat>(
            osc2FineId, "Osc 2 Fine",
            juce::NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("ct")),

        std::make_unique<juce::AudioParameterFloat>(
            oscMixId, "Osc Mix",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([] (float mix, int)
                {
                    const auto osc2Percent = juce::roundToInt (mix * 100.0f);
                    return juce::String (100 - osc2Percent) + " / " + juce::String (osc2Percent);
                })
                .withValueFromStringFunction ([] (const juce::String& text)
                {
                    return text.fromLastOccurrenceOf ("/", false, false).getFloatValue() / 100.0f;
                })),

        std::make_unique<juce::AudioParameterFloat>(
            envAttackId, "Attack",
            juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.3f), 0.005f,
            juce::AudioParameterFloatAttributes().withLabel ("s")),
        std::make_unique<juce::AudioParameterFloat>(
            envDecayId, "Decay",
            juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.3f), 0.1f,
            juce::AudioParameterFloatAttributes().withLabel ("s")),
        std::make_unique<juce::AudioParameterFloat>(
            envSustainId, "Sustain",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.8f),
        std::make_unique<juce::AudioParameterFloat>(
            envReleaseId, "Release",
            juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.3f), 0.2f,
            juce::AudioParameterFloatAttributes().withLabel ("s")),
        std::make_unique<juce::AudioParameterFloat>(
            envAttackCurveId, "Attack Curve",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            envDecayCurveId, "Decay Curve",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            envReleaseCurveId, "Release Curve",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f),

        std::make_unique<juce::AudioParameterFloat>(
            saturationDriveId, "Drive",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.2f),
        std::make_unique<juce::AudioParameterFloat>(
            saturationDriveEnvId, "Drive Env",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f),

        std::make_unique<juce::AudioParameterFloat>(
            glideTimeId, "Glide",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f, 0.3f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("s")),
        std::make_unique<juce::AudioParameterChoice>(
            glideModeId, "Glide Mode", juce::StringArray { "Time", "Rate" }, 1),

        std::make_unique<juce::AudioParameterFloat>(
            pitchBendId, "Pitch Bend",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f),
        std::make_unique<juce::AudioParameterInt>(
            pitchBendRangeId, "Bend Range", 0, 24, 2,
            juce::AudioParameterIntAttributes().withLabel ("st"))
    };
}
