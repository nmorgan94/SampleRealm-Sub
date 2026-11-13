#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <samplerealm/PeakMeter.h>
#include "Parameters.h"
#include "PitchBendInput.h"
#include "dsp/SubSynth.h"
#include <editor/editor.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    PresetManager& getPresetManager() { return presetManager; }
    PeakMeter& getOutputMeter() { return outputMeter; }

    static PresetManager::Config createPresetConfig();

private:
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters",
                                               createParameterLayout (ParameterSpecs::all, Parameters::versionHint,
                                                                      Parameters::withValueText) };
    PresetManager presetManager { apvts, createPresetConfig() };

    SubSynth synth;

    PitchBendInput pitchBendInput { *apvts.getParameter (Parameters::pitchBendId.getParamID()) };

    // Both channels carry the same mono signal
    PeakMeter outputMeter { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
