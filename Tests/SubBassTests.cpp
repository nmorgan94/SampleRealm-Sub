#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

namespace
{
    template <typename ParameterType>
    ParameterType* getTyped (AudioPluginAudioProcessor& processor, const char* paramID)
    {
        auto* parameter = dynamic_cast<ParameterType*> (processor.getAPVTS().getParameter (paramID));
        REQUIRE (parameter != nullptr);
        return parameter;
    }

    // Plays the same legato phrase for the same length at any block size, so tiny blocks still
    // get past the attack, glide into the second note and reach the release
    void playPhrase (AudioPluginAudioProcessor& processor, int numSamples)
    {
        constexpr int totalSamples = blockSize * 8;
        constexpr int glideSample = totalSamples / 4;
        constexpr int releaseSample = totalSamples / 2;
        constexpr int firstNote = 36;
        constexpr int secondNote = 48;

        // Both notes are released, because the voice only releases once no notes are held
        juce::MidiBuffer phrase;
        phrase.addEvent (juce::MidiMessage::noteOn (1, firstNote, 1.0f), 0);
        phrase.addEvent (juce::MidiMessage::noteOn (1, secondNote, 1.0f), glideSample);
        phrase.addEvent (juce::MidiMessage::noteOff (1, secondNote), releaseSample);
        phrase.addEvent (juce::MidiMessage::noteOff (1, firstNote), releaseSample);

        int firstNonFiniteBlockStart = -1;

        for (int start = 0; start < totalSamples; start += numSamples)
        {
            juce::AudioBuffer<float> buffer (2, numSamples);
            juce::MidiBuffer midi;
            midi.addEvents (phrase, start, numSamples, -start);

            processor.processBlock (buffer, midi);

            if (firstNonFiniteBlockStart < 0 && ! allSamplesFinite (buffer))
                firstNonFiniteBlockStart = start;
        }

        // One check per run, still naming the block where the output first went bad
        CAPTURE (firstNonFiniteBlockStart);
        CHECK (firstNonFiniteBlockStart == -1);
    }
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Every parameter starts at its default", "[parameters]")
{
    for (auto* parameter : processor.getParameters())
    {
        CAPTURE (parameter->getName (100));
        CHECK_THAT (parameter->getValue(), WithinAbs (parameter->getDefaultValue(), 1.0e-6));
        CHECK (parameter->getDefaultValue() >= 0.0f);
        CHECK (parameter->getDefaultValue() <= 1.0f);
    }
}

TEST_CASE_METHOD (ProcessorFixture, "Parameters have the expected ranges and defaults", "[parameters]")
{
    SECTION ("Float parameters")
    {
        const auto [paramID, minimum, maximum, defaultValue] = GENERATE (table<const char*, float, float, float> ({
            { "masterGain",         -60.0f,  6.0f,  0.0f   },
            { "osc1Fine",           -50.0f, 50.0f,  0.0f   },
            { "osc2Fine",           -50.0f, 50.0f,  0.0f   },
            { "oscMix",               0.0f,  1.0f,  0.5f   },
            { "envAttack",            0.001f, 5.0f, 0.005f },
            { "envDecay",             0.001f, 5.0f, 0.1f   },
            { "envSustain",           0.0f,  1.0f,  0.8f   },
            { "envRelease",           0.001f, 5.0f, 0.2f   },
            { "envAttackCurve",      -1.0f,  1.0f,  0.0f   },
            { "envDecayCurve",       -1.0f,  1.0f,  0.0f   },
            { "envReleaseCurve",     -1.0f,  1.0f,  0.0f   },
            { "saturationDrive",      0.0f,  1.0f,  0.2f   },
            { "saturationDriveEnv",  -1.0f,  1.0f,  0.0f   },
            { "glideTime",            0.0f,  1.0f,  0.0f   },
            { "pitchBend",           -1.0f,  1.0f,  0.0f   },
        }));

        CAPTURE (paramID);
        auto* parameter = getTyped<juce::AudioParameterFloat> (processor, paramID);
        CHECK_THAT (parameter->range.start, WithinAbs (minimum, 1.0e-6));
        CHECK_THAT (parameter->range.end, WithinAbs (maximum, 1.0e-6));
        CHECK_THAT (parameter->get(), WithinAbs (defaultValue, 1.0e-6));
    }

    SECTION ("Integer parameters")
    {
        auto* octave = getTyped<juce::AudioParameterInt> (processor, "oscOctave");
        CHECK (octave->getRange() == juce::Range<int> (-4, 0));
        CHECK (octave->get() == -2);

        auto* bendRange = getTyped<juce::AudioParameterInt> (processor, "pitchBendRange");
        CHECK (bendRange->getRange() == juce::Range<int> (0, 24));
        CHECK (bendRange->get() == 2);
    }

    SECTION ("Glide Mode is a Time/Rate choice defaulting to Rate")
    {
        auto* parameter = getTyped<juce::AudioParameterChoice> (processor, "glideMode");
        CHECK (parameter->choices == juce::StringArray { "Time", "Rate" });
        CHECK (parameter->getIndex() == 1);
    }
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Only mono or stereo output layouts are supported", "[buses]")
{
    using Set = juce::AudioChannelSet;

    // A synth has no input bus, so each layout is { {}, { output bus } }
    CHECK (processor.isBusesLayoutSupported ({ {}, { Set::mono() } }));
    CHECK (processor.isBusesLayoutSupported ({ {}, { Set::stereo() } }));

    CHECK_FALSE (processor.isBusesLayoutSupported ({ {}, { Set::create5point1() } }));
    CHECK_FALSE (processor.isBusesLayoutSupported ({ {}, { Set::quadraphonic() } }));
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "State round trips into a new instance", "[state]")
{
    setParameter (processor, "masterGain", -12.0f);
    setParameter (processor, "envAttack", 1.5f);
    setParameter (processor, "envReleaseCurve", -0.75f);
    setParameter (processor, "oscOctave", -4.0f);
    setParameter (processor, "glideMode", 0.0f);
    processor.getAPVTS().state.setProperty (PresetManager::currentPresetProperty, "Deep Sub", nullptr);

    juce::MemoryBlock state;
    processor.getStateInformation (state);

    AudioPluginAudioProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    CHECK_THAT (getParameter (restored, "masterGain"), WithinAbs (-12.0, 1.0e-4));
    CHECK_THAT (getParameter (restored, "envAttack"), WithinAbs (1.5, 1.0e-3));
    CHECK_THAT (getParameter (restored, "envReleaseCurve"), WithinAbs (-0.75, 1.0e-4));
    CHECK_THAT (getParameter (restored, "oscOctave"), WithinAbs (-4.0, 1.0e-6));
    CHECK_THAT (getParameter (restored, "glideMode"), WithinAbs (0.0, 1.0e-6));

    // The preset name lives in the ValueTree rather than in a parameter
    CHECK (restored.getPresetManager().getCurrentPresetName() == "Deep Sub");
}

TEST_CASE_METHOD (ProcessorFixture, "Invalid state is ignored", "[state]")
{
    setParameter (processor, "masterGain", -3.0f);
    setParameter (processor, "pitchBendRange", 7.0f);
    processor.getAPVTS().state.setProperty (PresetManager::currentPresetProperty, "Keep Me", nullptr);

    SECTION ("Random bytes")
    {
        const char garbage[] = "definitely not plugin state";
        processor.setStateInformation (garbage, static_cast<int> (sizeof (garbage)));
    }

    SECTION ("Empty data")
    {
        processor.setStateInformation (nullptr, 0);
    }

    SECTION ("Valid XML with the wrong root tag")
    {
        juce::MemoryBlock state;
        juce::AudioProcessor::copyXmlToBinary (juce::XmlElement ("SomethingElse"), state);
        processor.setStateInformation (state.getData(), static_cast<int> (state.getSize()));
    }

    CHECK_THAT (getParameter (processor, "masterGain"), WithinAbs (-3.0, 1.0e-4));
    CHECK_THAT (getParameter (processor, "pitchBendRange"), WithinAbs (7.0, 1.0e-6));
    CHECK (processor.getPresetManager().getCurrentPresetName() == "Keep Me");
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Output stays finite across block sizes and settings", "[dsp]")
{
    // Includes a block larger than the size given to prepareToPlay, which some hosts send
    const auto numSamples = GENERATE (1, 64, blockSize, blockSize * 4);
    CAPTURE (numSamples);

    SECTION ("Defaults")
    {
        playPhrase (processor, numSamples);
    }

    SECTION ("Drive and output gain at maximum")
    {
        const auto driveEnv = GENERATE (-1.0f, 0.0f, 1.0f);
        CAPTURE (driveEnv);

        setParameter (processor, "saturationDrive", 1.0f);
        setParameter (processor, "saturationDriveEnv", driveEnv);
        setParameter (processor, "masterGain", 6.0f);
        playPhrase (processor, numSamples);
    }

    SECTION ("Envelope curves at extremes over short and long stages")
    {
        const auto curve = GENERATE (-1.0f, 1.0f);
        const auto stageSeconds = GENERATE (0.001f, 0.5f);
        CAPTURE (curve, stageSeconds);

        for (const auto* paramID : { "envAttackCurve", "envDecayCurve", "envReleaseCurve" })
            setParameter (processor, paramID, curve);

        for (const auto* paramID : { "envAttack", "envDecay", "envRelease" })
            setParameter (processor, paramID, stageSeconds);

        // A mid sustain keeps decay and release ramping, so the curves shape a real span
        setParameter (processor, "envSustain", 0.5f);
        playPhrase (processor, numSamples);
    }

    SECTION ("Glide at maximum in both modes")
    {
        const auto glideMode = GENERATE (0.0f, 1.0f);
        CAPTURE (glideMode);

        setParameter (processor, "glideTime", 1.0f);
        setParameter (processor, "glideMode", glideMode);
        playPhrase (processor, numSamples);
    }

    SECTION ("Pitch bend at the edge of the widest range")
    {
        const auto bend = GENERATE (-1.0f, 1.0f);
        CAPTURE (bend);

        setParameter (processor, "pitchBendRange", 24.0f);
        setParameter (processor, "pitchBend", bend);
        setParameter (processor, "oscOctave", -4.0f);
        playPhrase (processor, numSamples);
    }

    SECTION ("Oscillators detuned and mixed hard to one side")
    {
        const auto mix = GENERATE (0.0f, 1.0f);
        CAPTURE (mix);

        setParameter (processor, "oscMix", mix);
        setParameter (processor, "osc1Fine", -50.0f);
        setParameter (processor, "osc2Fine", 50.0f);
        playPhrase (processor, numSamples);
    }
}
