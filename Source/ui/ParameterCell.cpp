#include "ParameterCell.h"

//==============================================================================
ParameterCell::ParameterCell (juce::Slider::SliderStyle style)
    : slider (style, juce::Slider::NoTextBox)
{
    addAndMakeVisible (slider);

    slider.onDragStart = [this]
    {
        stopTimer();
        isDragging = true;
        showCurrentValue();
    };
    slider.onValueChange = [this]
    {
        if (isDragging)
            showCurrentValue();
    };
    slider.onDragEnd = [this]
    {
        isDragging = false;
        startTimer (revertDelayMs);
    };

    slider.addMouseListener (this, false);

    label.setFont (juce::FontOptions (nameHeight, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.addListener (this);
    addAndMakeVisible (label);
}

void ParameterCell::attachToParameter (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);

    slider.setDoubleClickReturnValue (false, 0.0);
}

void ParameterCell::parentHierarchyChanged()
{
    // The slider bakes in look-and-feel-dependent child components (e.g. IncDecButtons)
    // at construction time, before this component is parented and can resolve the real
    // look and feel. Force it to rebuild them now that the ancestor chain is in place.
    slider.sendLookAndFeelChange();
}

void ParameterCell::mouseDoubleClick (const juce::MouseEvent&)
{
    stopTimer();
    isDragging = false;
    showCurrentValue();
    label.showEditor();
}

void ParameterCell::editorHidden (juce::Label*, juce::TextEditor& editor)
{
    slider.setValue (slider.getValueFromText (editor.getText()), juce::sendNotificationSync);
    showName();
}

void ParameterCell::setLabelText (const juce::String& newText)
{
    nameText = newText;
    showName();
}

void ParameterCell::setAccessory (juce::Component& component, int width, int height)
{
    accessory = &component;
    accessoryWidth = width;
    accessoryHeight = height;

    addAndMakeVisible (component);
    resized();
}

void ParameterCell::timerCallback()
{
    stopTimer();
    showName();
}

void ParameterCell::showCurrentValue()
{
    label.setText (slider.getTextFromValue (slider.getValue()), juce::dontSendNotification);
}

void ParameterCell::showName()
{
    label.setText (nameText, juce::dontSendNotification);
}

void ParameterCell::resized()
{
    auto bounds = getLocalBounds();

    if (accessory != nullptr)
        accessory->setBounds (bounds.removeFromBottom (accessoryHeight).withSizeKeepingCentre (accessoryWidth, accessoryHeight));

    label.setBounds (bounds.removeFromBottom (labelHeight));

    if (slider.getSliderStyle() == juce::Slider::IncDecButtons)
        bounds = bounds.withSizeKeepingCentre (bounds.getWidth(), incDecButtonsHeight);

    slider.setBounds (bounds);
}
