#include "MasterSection.h"
#include "CustomLookAndFeel.h"

MasterSection::MasterSection (juce::AudioProcessorValueTreeState& apvts, const juce::String& gainParamId,
                              juce::Component& popupParent)
    : gainAttachment (apvts, gainParamId, gainKnob)
{
    gainKnob.setPopupDisplayEnabled (true, true, &popupParent);

    addAndMakeVisible (gainKnob);
    addAndMakeVisible (meter);
}

void MasterSection::setLevel (float linearLevel, bool isClipping)
{
    meter.setLevel (linearLevel);
    meter.setClipping (isClipping);
}

void MasterSection::paint (juce::Graphics& g)
{
    g.setColour (CustomLookAndFeel::textDim);
    g.setFont (CustomLookAndFeel::boldFont (10.0f));
    g.drawText ("MASTER", getLocalBounds().withWidth (labelWidth), juce::Justification::centredRight);
}

void MasterSection::resized()
{
    auto bounds = getLocalBounds();

    meter.setBounds (bounds.removeFromRight (meterWidth).withSizeKeepingCentre (meterWidth, meterHeight));
    bounds.removeFromRight (meterGap);
    gainKnob.setBounds (bounds.removeFromRight (knobSize).withSizeKeepingCentre (knobSize, knobSize));
}
