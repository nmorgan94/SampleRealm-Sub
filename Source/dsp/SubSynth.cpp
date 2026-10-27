#include "SubSynth.h"
#include <algorithm>

//==============================================================================
void SubSynth::prepare (double sampleRate)
{
    voice.prepare (sampleRate);
}

void SubSynth::setSettings (const Settings& newSettings)
{
    masterGainDb = newSettings.masterGainDb;
    voice.setParameters (newSettings.voice);
}

void SubSynth::setPitchBend (float normalisedBend)
{
    voice.setPitchBend (normalisedBend);
}

void SubSynth::process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi)
{
    buffer.clear();

    int currentSample = 0;

    for (const auto metadata : midi)
    {
        if (metadata.samplePosition > currentSample)
        {
            voice.renderNextBlock (buffer, currentSample, metadata.samplePosition - currentSample);
            currentSample = metadata.samplePosition;
        }

        handleMidiEvent (metadata.getMessage());
    }

    voice.renderNextBlock (buffer, currentSample, buffer.getNumSamples() - currentSample);

    const float gainLinear = juce::Decibels::decibelsToGain (masterGainDb, -60.0f);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        juce::FloatVectorOperations::multiply (buffer.getWritePointer (channel), gainLinear, buffer.getNumSamples());
}

void SubSynth::handleMidiEvent (const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        const auto note = message.getNoteNumber();
        removeHeldNote (note);
        heldNotes.push_back (note);
        voice.noteOn (note, heldNotes.size() == 1);
    }
    else if (message.isNoteOff())
    {
        removeHeldNote (message.getNoteNumber());

        if (heldNotes.empty())
            voice.noteOff();
        else
            voice.noteOn (heldNotes.back(), false);
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        heldNotes.clear();
        voice.noteOff();
    }
    else if (message.isPitchWheel())
        voice.setPitchBend (juce::MPEValue::from14BitInt (message.getPitchWheelValue()).asSignedFloat());
}

void SubSynth::removeHeldNote (int note)
{
    heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
}
