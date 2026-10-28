#include <juce_audio_basics/juce_audio_basics.h>
#include "shared/ParameterSpecs.h"
#include "shared/SynthSettings.h"
#include "shared/UiInfoJson.h"
#include "dsp/Curve.h"
#include "dsp/DriveEnvelopeModulation.h"
#include "dsp/SubSynth.h"

//==============================================================================
namespace
{
    constexpr int maxBlockSize = 128;

    constexpr auto numParams = static_cast<int> (ParameterSpecs::all.size());

    constexpr auto indexOf (const ParameterSpecs::Spec& spec)
    {
        return std::ranges::find (ParameterSpecs::all, &spec) - ParameterSpecs::all.begin();
    }

    auto defaultValues()
    {
        std::array<float, ParameterSpecs::all.size()> values;

        for (size_t i = 0; i < values.size(); ++i)
            values[i] = ParameterSpecs::all[i]->defaultValue;

        return values;
    }

    auto values = defaultValues();

    float valueOf (const ParameterSpecs::Spec& spec)
    {
        return values[static_cast<size_t> (indexOf (spec))];
    }

    SubSynth synth;
    juce::AudioBuffer<float> buffer { 1, maxBlockSize };

    // Played at the start of the next block
    juce::MidiBuffer pendingMidi;
}

#define EXPORT(name) extern "C" __attribute__ ((export_name (#name)))

EXPORT (uiInfo) const char* uiInfo()
{
    static const auto json = uiInfoJson (SUB_PLUGIN_VERSION);
    return json.c_str();
}

// For webui/'s tests
EXPORT (shapeProgress) float exportedShapeProgress (float progress, float tension) { return shapeProgress (progress, tension); }
EXPORT (driveEnvelopeModulation) float exportedDriveEnvelopeModulation (float env, float amount) { return driveEnvelopeModulation (env, amount); }

EXPORT (prepare) void prepare (double sampleRate)
{
    synth.prepare (sampleRate);
}

EXPORT (setParam) void setParam (int index, float value)
{
    if (index < 0 || index >= numParams)
        return;

    values[static_cast<size_t> (index)] = value;

    if (index == indexOf (ParameterSpecs::pitchBend))
        synth.setPitchBend (value);
}

EXPORT (midi) void midi (int status, int data1, int data2)
{
    if (status < 0x80 || status >= 0xf0)
        return;

    pendingMidi.addEvent (juce::MidiMessage (status, data1 & 0x7f, data2 & 0x7f), 0);
}

EXPORT (render) float* render (int numSamples)
{
    buffer.setSize (1, std::clamp (numSamples, 0, maxBlockSize), false, false, true);

    synth.setSettings (makeSynthSettings (valueOf));
    synth.process (buffer, pendingMidi);
    pendingMidi.clear();

    return buffer.getWritePointer (0);
}
