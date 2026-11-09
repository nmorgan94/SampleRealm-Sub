#pragma once

#include "PluginProcessor.h"
#include "ui/About.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/EnvelopeVisualizer.h"
#include "ui/HeaderBar.h"
#include "ui/LabeledSlider.h"
#include "ui/Manual.h"
#include "ui/MasterSection.h"
#include "ui/PillButton.h"
#include "ui/PitchWheel.h"
#include "ui/PromptOverlay.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    static constexpr int width = 700, height = 640;

    // As HEADER_LAYOUT in webui/src/SubEditor.tsx
    static constexpr HeaderBar::Layout headerLayout { .height = 60, .edge = 20, .titleWidth = 200, .titleHeight = 22.0f,
                                                      .gap = 20, .controlsHeight = 24, .meterWidth = MasterSection::preferredWidth, .meterInset = 8 };

    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void addCurveKnob (LabeledSlider& owner, const juce::String& paramId);

    LabeledSlider& addControl (const juce::String& paramId, const juce::String& labelText,
                               juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    CustomLookAndFeel customLookAndFeel;

    std::vector<std::unique_ptr<LabeledSlider>> controls;

    std::vector<std::unique_ptr<juce::Slider>> curveKnobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> curveKnobAttachments;

    EnvelopeVisualizer envelopeVisualizer;

    PillButton glideModeButton { "TIME", "RATE" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> glideModeAttachment;
    PitchWheel pitchWheel;

    PromptOverlay prompt;
    ManualOverlay manual;
    MasterSection masterSection { processorRef.getAPVTS(), Parameters::masterGainId.getParamID(), *this };
    HeaderBar header { "SampleRealm: SUB", headerLayout, processorRef.getPresetManager(), prompt, [this] { manual.show(); },
                       About::text (processorRef), masterSection };

    static constexpr int numColumns = 4;
    static constexpr int numRows = 3;
    static constexpr int envelopeRowIndex = 1;
    static constexpr int outputRowIndex = 2;

    std::array<juce::String, numRows> rowTitles { "OSCILLATOR", "ENVELOPE", "TONE & PITCH" };
    std::array<juce::Rectangle<int>, numRows> rowBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
