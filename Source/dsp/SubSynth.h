#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include "SubBassVoice.h"

//==============================================================================
class SubSynth
{
public:
    struct Settings
    {
        SubBassVoice::Params voice;
        float masterGainDb = 0.0f;
    };

    void prepare (double sampleRate);
    void setSettings (const Settings& newSettings);

    // -1..1
    void setPitchBend (float normalisedBend);

    void process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);

private:
    void handleMidiEvent (const juce::MidiMessage& message);
    void removeHeldNote (int note);

    SubBassVoice voice;
    std::vector<int> heldNotes;
    float masterGainDb = 0.0f;
};
