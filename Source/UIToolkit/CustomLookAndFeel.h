/*
  ==============================================================================
    CustomLookAndFeel.h - Dark modern theme: color palette + LookAndFeel
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace gui
{

/** Color palette for the dark theme. */
namespace Colors
{
    static const juce::Colour background       { 0xff1A1A2E };
    static const juce::Colour panelBackground  { 0xff16213E };
    static const juce::Colour panelBorder      { 0xff0F3460 };
    static const juce::Colour accent           { 0xffE94560 };
    static const juce::Colour accentDark       { 0xffA83279 };
    static const juce::Colour textBright       { 0xffEEEEEE };
    static const juce::Colour textDim          { 0xff8899AA };
    static const juce::Colour knobBackground   { 0xff0A0E1A };
    static const juce::Colour knobArcTrack     { 0xff2A2E4A };
    static const juce::Colour knobArcActive    { 0xffE94560 };
    static const juce::Colour knobThumb        { 0xffFFFFFF };
    static const juce::Colour waveformGreen    { 0xff00E676 };
    static const juce::Colour spectrumCyan     { 0xff00BCD4 };
    static const juce::Colour envelopeYellow   { 0xffFFD54F };
}

/**
 * Custom LookAndFeel based on V4 with dark theme styling.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Set the dark colour scheme
        setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour(juce::Label::textColourId, Colors::textBright);
        setColour(juce::Slider::textBoxTextColourId, Colors::textBright);
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::knobBackground);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonColourId, Colors::panelBorder);
        setColour(juce::TextButton::textColourOffId, Colors::textBright);
        setColour(juce::TextButton::buttonOnColourId, Colors::accent);
        setColour(juce::TextButton::textColourOnId, Colors::textBright);
        setColour(juce::ComboBox::backgroundColourId, Colors::panelBackground);
        setColour(juce::ComboBox::textColourId, Colors::textBright);
        setColour(juce::ComboBox::outlineColourId, Colors::panelBorder);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& /*slider*/) override
    {
        const float radius = static_cast<float>(juce::jmin(width, height)) * 0.4f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float angle = rotaryStartAngle
                            + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background arc
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::knobArcTrack);
        g.strokePath(bgArc, juce::PathStrokeType(3.0f));

        // Active arc
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                                rotaryStartAngle, angle, true);
        g.setColour(Colors::knobArcActive);
        g.strokePath(activeArc, juce::PathStrokeType(3.0f));

        // Thumb
        juce::Path thumb;
        const float thumbLen = radius * 0.3f;
        thumb.addRectangle(-1.5f, -radius, 3.0f, thumbLen);
        g.setColour(Colors::knobThumb);
        g.fillPath(thumb, juce::AffineTransform::rotation(angle)
                              .translated(centreX, centreY));
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());

        const auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(textArea.getHeight())
                                                         / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto col = backgroundColour;

        if (shouldDrawButtonAsDown)
            col = col.brighter(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            col = col.brighter(0.1f);

        g.setColour(col);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool /*isButtonDown*/, int /*buttonX*/, int /*buttonY*/,
                      int /*buttonW*/, int /*buttonH*/,
                      juce::ComboBox& box) override
    {
        const auto bounds = juce::Rectangle<float>(0.0f, 0.0f,
                            static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
        const float cornerSize = 5.0f;

        // Background
        g.setColour(Colors::knobBackground);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.drawRoundedRectangle(bounds, cornerSize, 1.2f);

        // Arrow chevron
        {
            const float arrowZone = 20.0f;
            const float arrowX = bounds.getRight() - arrowZone - 4.0f;
            const float arrowCY = bounds.getCentreY();
            const float arrowHalf = 4.0f;

            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowCY - arrowHalf * 0.5f);
            arrow.lineTo(arrowX + arrowHalf, arrowCY + arrowHalf * 0.5f);
            arrow.lineTo(arrowX + arrowHalf * 2.0f, arrowCY - arrowHalf * 0.5f);

            g.setColour(Colors::textDim);
            g.strokePath(arrow, juce::PathStrokeType(1.5f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // ── Popup Menu ──────────────────────────────────────────────

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        const auto bounds = juce::Rectangle<float>(0.0f, 0.0f,
                            static_cast<float>(width), static_cast<float>(height));

        // Square corners — avoids white-corner artefacts from the opaque OS window
        g.setColour(Colors::panelBackground);
        g.fillRect(bounds);

        g.setColour(Colors::panelBorder);
        g.drawRect(bounds.reduced(0.5f), 1.0f);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& /*shortcutKeyText*/,
                           const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/) override
    {
        if (isSeparator)
        {
            auto sepArea = area.reduced(8, 0).withSizeKeepingCentre(area.getWidth() - 16, 1);
            g.setColour(Colors::panelBorder);
            g.fillRect(sepArea);
            return;
        }

        auto r = area.reduced(2, 0);

        if (isHighlighted && isActive)
        {
            g.setColour(Colors::accent.withAlpha(0.25f));
            g.fillRoundedRectangle(r.toFloat(), 4.0f);
        }

        g.setColour(isActive ? (isHighlighted ? Colors::textBright : Colors::textBright)
                             : Colors::textDim);
        g.setFont(juce::FontOptions(13.0f));

        const int tickW = 24;
        auto textArea = r.withTrimmedLeft(tickW);

        if (isTicked)
        {
            // Draw a small check mark
            const float cx = static_cast<float>(r.getX()) + 7.0f;
            const float cy = static_cast<float>(r.getCentreY());

            juce::Path tick;
            tick.startNewSubPath(cx, cy);
            tick.lineTo(cx + 3.5f, cy + 3.5f);
            tick.lineTo(cx + 9.0f, cy - 3.5f);

            g.setColour(Colors::accent);
            g.strokePath(tick, juce::PathStrokeType(2.0f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);

        if (hasSubMenu)
        {
            const float arrowH = 6.0f;
            const float arrowX = static_cast<float>(r.getRight()) - 12.0f;
            const float arrowY = static_cast<float>(r.getCentreY());

            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowY - arrowH * 0.5f);
            arrow.lineTo(arrowX + arrowH * 0.5f, arrowY);
            arrow.lineTo(arrowX, arrowY + arrowH * 0.5f);

            g.setColour(Colors::textDim);
            g.strokePath(arrow, juce::PathStrokeType(1.5f));
        }
    }

    int getPopupMenuBorderSize() override { return 1; }
};

} // namespace gui
