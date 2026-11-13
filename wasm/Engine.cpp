#include <juce_audio_basics/juce_audio_basics.h>
#include <samplerealm/BuildCommit.h>
#include <samplerealm/ParameterValues.h>
#include <samplerealm/PeakMeter.h>
#include <samplerealm/UiInfoJson.h>
#include <samplerealm/WasmExport.h>
#include "shared/ParameterSpecs.h"
#include "shared/SynthSettings.h"
#include "dsp/Curve.h"
#include "dsp/DriveEnvelopeModulation.h"
#include "dsp/SubSynth.h"

//==============================================================================
namespace
{
    constexpr int maxBlockSize = 128;

    ParameterValues values { ParameterSpecs::all };

    SubSynth synth;
    PeakMeter meter { 1 };
    // Mono: the page copies the one channel to each of its outputs
    juce::AudioBuffer<float> buffer { 1, maxBlockSize };

    // Played at the start of the next block
    juce::MidiBuffer pendingMidi;

    constexpr auto pitchBendIndex = std::ranges::find (ParameterSpecs::all, &ParameterSpecs::pitchBend) - ParameterSpecs::all.begin();
}

WASM_EXPORT (uiInfo) const char* uiInfo()
{
    static const auto json = uiInfoJson (ParameterSpecs::all, PLUGIN_VERSION, buildCommit);
    return json.c_str();
}

// For webui/'s curve tests
WASM_EXPORT (shapeProgress) float exportedShapeProgress (float progress, float tension) { return shapeProgress (progress, tension); }
WASM_EXPORT (driveEnvelopeModulation) float exportedDriveEnvelopeModulation (float env, float amount) { return driveEnvelopeModulation (env, amount); }

// Always one channel, whatever the page asks for
WASM_EXPORT (prepare) void prepare (double sampleRate, int)
{
    synth.prepare (sampleRate);
}

WASM_EXPORT (setParam) void setParam (int index, float value)
{
    values.set (index, value);

    // As PitchBendInput: the parameter moves the bend too, not only the wheel
    if (index == pitchBendIndex)
        synth.setPitchBend (values (ParameterSpecs::pitchBend));
}

WASM_EXPORT (midi) void midi (int status, int data1, int data2)
{
    if (status < 0x80 || status >= 0xf0)
        return;

    pendingMidi.addEvent (juce::MidiMessage (status, data1 & 0x7f, data2 & 0x7f), 0);
}

// The output is read back from here after render, for any channel the page asks for
WASM_EXPORT (channel) float* channel (int)
{
    return buffer.getWritePointer (0);
}

WASM_EXPORT (render) void render (int numSamples)
{
    juce::AudioBuffer<float> block { buffer.getArrayOfWritePointers(), 1, std::clamp (numSamples, 0, maxBlockSize) };

    synth.setSettings (makeSynthSettings (values));
    synth.process (block, pendingMidi);
    pendingMidi.clear();
    meter.process (block);
}

// The loudest output sample since the last call, as linear gain
WASM_EXPORT (peak) float peak (int index)
{
    return index == 0 ? meter.readAndReset (0) : 0.0f;
}
