#pragma once

#include "LevelMeter.h"
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 * The MASTER label, gain knob and output meter, in the header's meter slot.
 * Mirrored by webui/src/MasterSection.tsx.
 */
class MasterSection : public juce::Component
{
public:
    static constexpr int labelWidth = 66, labelGap = 8, knobSize = 44, meterGap = 10, meterWidth = 14, meterHeight = 40;
    static constexpr int preferredWidth = labelWidth + labelGap + knobSize + meterGap + meterWidth;

    MasterSection (juce::AudioProcessorValueTreeState&, const juce::String& gainParamId, juce::Component& popupParent);

    void setLevel (float linearLevel, bool isClipping);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider gainKnob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;
    LevelMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterSection)
};
