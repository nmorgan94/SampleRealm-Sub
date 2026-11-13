#include "CustomLookAndFeel.h"
#include <BinaryData.h>

namespace
{
    // On the stepper's +/- buttons, which keep LookAndFeel_V4's font, as webui/src/controls.tsx's IncDecButtons
    const juce::Identifier isSliderButton { "isSliderButton" };
}

//==============================================================================
CustomLookAndFeel::CustomLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, background);
    setColour (juce::Slider::rotarySliderFillColourId, accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, panelBorder);
    setColour (juce::Slider::thumbColourId, accent);
    setColour (juce::Slider::textBoxTextColourId, text);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    // The web's --color-fg, which the shared web overlays use
    setColour (juce::Label::textColourId, text);

    setColour (juce::ComboBox::outlineColourId, panelBorder);
    setColour (juce::TextButton::buttonColourId, panel);
    setColour (juce::TextButton::textColourOffId, text);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha (0.3f));

    // The editor's gradient shows through
    setColour (HeaderBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour (HeaderBar::titleColourId, text);

    setColour (ModalOverlay::dimColourId, background.withAlpha (0.75f));
    setColour (ModalOverlay::panelColourId, panel);
    setColour (ModalOverlay::outlineColourId, panelBorder);
    setColour (LevelMeter::backgroundColourId, panel);
    setColour (LevelMeter::outlineColourId, panelBorder);
    setColour (LevelMeter::barColourId, accent);
    setColour (LevelMeter::clipColourId, juce::Colours::red);
    setColour (juce::TextEditor::backgroundColourId, background);
    setColour (juce::TextEditor::textColourId, text);
    setColour (juce::TextEditor::outlineColourId, panelBorder);
    setColour (juce::TextEditor::focusedOutlineColourId, accent);
    setColour (juce::TextEditor::highlightColourId, accent.withAlpha (0.4f));
    setColour (juce::CaretComponent::caretColourId, accent);
    setColour (juce::ScrollBar::thumbColourId, accent);
}

juce::Font CustomLookAndFeel::getLabelFont (juce::Label& label)
{
    const auto font = label.getFont();
    return getEditorFont (font.getHeight(), font.isBold());
}

juce::Font CustomLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    if (button.getProperties()[isSliderButton])
        return LookAndFeel_V4::getTextButtonFont (button, buttonHeight);

    return getEditorFont (12.0f, false);
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return getEditorFont (13.0f, false);
}

juce::Font CustomLookAndFeel::getEditorFont (float height, bool bold)
{
    return bold ? boldFont (height) : juce::Font (orbitronRegular()).withHeight (height);
}

juce::Font CustomLookAndFeel::boldFont (float height)
{
    return juce::Font (orbitronBold()).withHeight (height);
}

juce::FontOptions CustomLookAndFeel::orbitronRegular()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::OrbitronRegular_ttf,
        BinaryData::OrbitronRegular_ttfSize);
    return juce::FontOptions (typeface);
}

juce::FontOptions CustomLookAndFeel::orbitronBold()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::OrbitronBold_ttf,
        BinaryData::OrbitronBold_ttfSize);
    return juce::FontOptions (typeface);
}

juce::Button* CustomLookAndFeel::createSliderButton (juce::Slider&, bool isIncrement)
{
    auto* button = new juce::TextButton (isIncrement ? "+" : "-");
    button->getProperties().set (isSliderButton, true);

    button->setColour (juce::TextButton::buttonColourId, panel);
    button->setColour (juce::TextButton::textColourOffId, accent);
    button->setColour (juce::TextButton::textColourOnId, accent);
    button->setColour (juce::ComboBox::outlineColourId, accent);
    button->setConnectedEdges (isIncrement ? juce::Button::ConnectedOnLeft : juce::Button::ConnectedOnRight);

    return button;
}

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                static_cast<float> (width), static_cast<float> (height))
                             .reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto sweep = rotaryEndAngle - rotaryStartAngle;

    constexpr int numSegments = 24;
    constexpr float gapRatio = 0.3f;
    const auto segmentSpan = sweep / static_cast<float> (numSegments);
    const auto ringThickness = radius * 0.16f;

    for (int i = 0; i < numSegments; ++i)
    {
        const auto segStart = rotaryStartAngle + segmentSpan * (static_cast<float> (i) + gapRatio * 0.5f);
        const auto segEnd = rotaryStartAngle + segmentSpan * (static_cast<float> (i + 1) - gapRatio * 0.5f);
        const auto segFraction = (static_cast<float> (i) + 0.5f) / static_cast<float> (numSegments);
        const bool lit = segFraction <= sliderPosProportional;

        juce::Path segment;
        segment.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, segStart, segEnd, true);

        if (lit)
        {
            g.setColour (accent.withAlpha (0.22f));
            g.strokePath (segment, juce::PathStrokeType (ringThickness + 5.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
        }

        g.setColour (lit ? accent : panelBorder);
        g.strokePath (segment, juce::PathStrokeType (ringThickness, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    }

    const auto knobRadius = radius - ringThickness - radius * 0.14f;

    juce::ColourGradient bodyGradient (panel.brighter (0.2f), centre.x, centre.y - knobRadius,
                                       panel.darker (0.5f), centre.x, centre.y + knobRadius, false);
    g.setGradientFill (bodyGradient);
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    constexpr int numRidges = 32;
    for (int i = 0; i < numRidges; ++i)
    {
        const auto ridgeAngle = juce::MathConstants<float>::twoPi * static_cast<float> (i) / static_cast<float> (numRidges);
        const auto ridge = juce::Line<float> (centre.getPointOnCircumference (knobRadius * 0.86f, ridgeAngle),
                                              centre.getPointOnCircumference (knobRadius * 0.98f, ridgeAngle));
        g.setColour (panelBorder.withAlpha (0.35f));
        g.drawLine (ridge, 1.0f);
    }

    g.setColour (panelBorder);
    g.drawEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

    const auto highlightRadius = knobRadius * 0.7f;
    juce::ColourGradient highlightGradient (juce::Colours::white.withAlpha (0.08f), centre.x, centre.y - knobRadius * 0.6f,
                                            juce::Colours::transparentWhite, centre.x, centre.y, false);
    g.setGradientFill (highlightGradient);
    g.fillEllipse (centre.x - highlightRadius, centre.y - knobRadius * 0.9f, highlightRadius * 2.0f, highlightRadius);

    const auto indicator = juce::Line<float> (centre.getPointOnCircumference (knobRadius * 0.15f, angle),
                                              centre.getPointOnCircumference (knobRadius * 0.82f, angle));
    g.setColour (text);
    g.drawLine (indicator, 2.2f);

    const auto tip = indicator.getEnd();
    g.setColour (accent);
    g.fillEllipse (tip.x - 2.6f, tip.y - 2.6f, 5.2f, 5.2f);
}

void CustomLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    constexpr float trackHeight = 4.0f;
    constexpr float capWidth = 14.0f;
    constexpr float capHeight = 28.0f;
    constexpr float centreTickHeight = 14.0f;

    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre = bounds.getCentre();
    const auto track = bounds.withSizeKeepingCentre (bounds.getWidth(), trackHeight);

    g.setColour (panelBorder);
    g.fillRoundedRectangle (track, trackHeight * 0.5f);

    g.setColour (textDim);
    g.fillRect (juce::Rectangle<float> (1.0f, centreTickHeight).withCentre (centre));

    const auto litTrack = track.withLeft (juce::jmin (centre.x, sliderPos)).withRight (juce::jmax (centre.x, sliderPos));
    g.setColour (accent.withAlpha (0.22f));
    g.fillRect (litTrack.expanded (0.0f, 2.5f));
    g.setColour (accent);
    g.fillRect (litTrack);

    const auto cap = juce::Rectangle<float> (capWidth, capHeight).withCentre ({ sliderPos, centre.y });

    juce::ColourGradient capGradient (panel.brighter (0.2f), cap.getX(), cap.getY(),
                                      panel.darker (0.5f), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (capGradient);
    g.fillRoundedRectangle (cap, 3.0f);

    g.setColour (panelBorder);
    g.drawRoundedRectangle (cap, 3.0f, 1.2f);

    g.setColour (accent);
    g.fillRect (juce::Rectangle<float> (2.0f, capHeight * 0.5f).withCentre (cap.getCentre()));
}
