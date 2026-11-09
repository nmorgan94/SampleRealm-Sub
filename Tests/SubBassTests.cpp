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
TEST_CASE_METHOD (ProcessorFixture, "Every parameter is built as its spec says and starts at its default", "[parameters]")
{
    for (const auto* spec : ParameterSpecs::all)
    {
        CAPTURE (spec->id);

        switch (spec->kind)
        {
            case ParameterSpecs::Kind::real:
            {
                auto* parameter = getTyped<juce::AudioParameterFloat> (processor, spec->id);
                CHECK_THAT (parameter->range.start, WithinAbs (spec->min, 1.0e-6));
                CHECK_THAT (parameter->range.end, WithinAbs (spec->max, 1.0e-6));
                CHECK_THAT (parameter->range.interval, WithinAbs (spec->step, 1.0e-6));
                CHECK_THAT (parameter->range.skew, WithinAbs (spec->skew, 1.0e-6));
                CHECK_THAT (parameter->get(), WithinAbs (spec->defaultValue, 1.0e-6));
                break;
            }

            case ParameterSpecs::Kind::integer:
            {
                auto* parameter = getTyped<juce::AudioParameterInt> (processor, spec->id);
                CHECK (parameter->getRange() == juce::Range<int> (juce::roundToInt (spec->min), juce::roundToInt (spec->max)));
                CHECK (parameter->get() == juce::roundToInt (spec->defaultValue));
                break;
            }

            case ParameterSpecs::Kind::choice:
            {
                juce::StringArray choices;
                for (const auto* choice : spec->choices)
                    choices.add (choice);

                auto* parameter = getTyped<juce::AudioParameterChoice> (processor, spec->id);
                CHECK (parameter->choices == choices);
                CHECK (parameter->getIndex() == juce::roundToInt (spec->defaultValue));
                break;
            }
        }
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
    restoreSessionShowing (processor, "Deep Sub", true);

    juce::MemoryBlock state;
    processor.getStateInformation (state);

    AudioPluginAudioProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    CHECK_THAT (getParameter (restored, "masterGain"), WithinAbs (-12.0, 1.0e-4));
    CHECK_THAT (getParameter (restored, "envAttack"), WithinAbs (1.5, 1.0e-3));
    CHECK_THAT (getParameter (restored, "envReleaseCurve"), WithinAbs (-0.75, 1.0e-4));
    CHECK_THAT (getParameter (restored, "oscOctave"), WithinAbs (-4.0, 1.0e-6));
    CHECK_THAT (getParameter (restored, "glideMode"), WithinAbs (0.0, 1.0e-6));

    const auto current = restored.getPresetManager().getCurrent();
    CHECK (current.name == "Deep Sub");
    CHECK_FALSE (current.isFactory);
    CHECK (restored.getPresetManager().isModified());

    // Kept by the preset manager, not left on the parameters
    CHECK_FALSE (restored.getAPVTS().state.hasProperty ("presetName"));
}

TEST_CASE_METHOD (ProcessorFixture, "Invalid state is ignored", "[state]")
{
    setParameter (processor, "masterGain", -3.0f);
    setParameter (processor, "pitchBendRange", 7.0f);
    restoreSessionShowing (processor, "Keep Me", false);

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
    CHECK (processor.getPresetManager().getCurrent().name == "Keep Me");
    CHECK_FALSE (processor.getPresetManager().isModified());
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
