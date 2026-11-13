#pragma once

#include <editor/editor.h>

//==============================================================================

class CustomLookAndFeel : public juce::LookAndFeel_V4,
                          public EditorFonts
{
public:
    static inline const juce::Colour background { 0xff0b0d11 };
    static inline const juce::Colour panel { 0xff14171d };
    static inline const juce::Colour panelBorder { 0xff262b33 };
    static inline const juce::Colour accent { 0xff2fe8c4 };
    static inline const juce::Colour text { 0xffd8dee6 };
    static inline const juce::Colour textDim { 0xff7a828e };

    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    juce::Button* createSliderButton (juce::Slider&, bool isIncrement) override;

    // Sizes as the shared web editor's: a label's own, buttons 12, the menu 13
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;

    // The header's title, the manual and the prompt's text field
    juce::Font getEditorFont (float height, bool bold) override;

    [[nodiscard]] static juce::Font boldFont (float height);
    [[nodiscard]] static juce::FontOptions orbitronRegular();

private:
    [[nodiscard]] static juce::FontOptions orbitronBold();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
