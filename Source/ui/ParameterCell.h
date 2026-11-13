#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
    A slider of any style with its name shown below it, for the controls the shared
    LabeledSlider can't show: steppers, a linear slider, and knobs with an accessory.
    While the slider is being dragged, the label swaps to show the current value
    instead; on release it reverts to the name. Double-clicking the slider lets you
    type an exact value into the label. Mirrored by LabeledCell in webui/src/controls.tsx.
*/
class ParameterCell : public juce::Component,
                      private juce::Timer,
                      private juce::Label::Listener
{
public:
    // The shared knobs beside it use the same, as NAME_HEIGHT in webui/src/SubEditor.tsx
    static constexpr float nameHeight = 13.0f;

    explicit ParameterCell (juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag);

    void attachToParameter (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId);
    void setLabelText (const juce::String& newText);

    /** Places a caller-owned component in a strip beneath the name label, centred at the given size. */
    void setAccessory (juce::Component& component, int width, int height);

    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void parentHierarchyChanged() override;

private:
    void showCurrentValue();
    void showName();
    void timerCallback() override;
    void labelTextChanged (juce::Label*) override {}
    void editorHidden (juce::Label*, juce::TextEditor&) override;

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::String nameText;
    bool isDragging = false;

    juce::Component* accessory = nullptr;
    int accessoryWidth = 0;
    int accessoryHeight = 0;

    static constexpr int labelHeight = 16;
    static constexpr int revertDelayMs = 600;
    static constexpr int incDecButtonsHeight = 70;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterCell)
};
