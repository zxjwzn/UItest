/*
  ==============================================================================
    SectionPanel.h - Rounded panel container with title
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * A container component that draws a rounded rectangle background
 * with a section title label.
 */
class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel(const juce::String& title)
        : sectionTitle(title)
    {
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(borderInset);

        // Panel background with rounded corners
        g.setColour(Colors::panelBackground);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // Subtle border
        g.setColour(Colors::panelBorder.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

        // Section title
        g.setColour(Colors::accent);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(sectionTitle,
                   bounds.reduced(contentPadX, contentPadY).removeFromTop(titleHeight),
                   juce::Justification::centredLeft);
    }

    /** Get the content area (below the title). */
    juce::Rectangle<int> getContentArea() const
    {
        return getLocalBounds()
            .reduced(static_cast<int>(contentPadX), static_cast<int>(contentPadY))
            .withTrimmedTop(static_cast<int>(titleHeight) + titleGap);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        // Click on panel background → release keyboard focus from any child
        unfocusAllComponents();
    }

private:
    juce::String sectionTitle;

    // ── Layout constants (single source of truth) ───────────────
    static constexpr float borderInset  = 1.0f;
    static constexpr float cornerRadius = 8.0f;
    static constexpr float contentPadX  = 12.0f;   // horizontal padding inside panel
    static constexpr float contentPadY  = 6.0f;    // vertical padding inside panel
    static constexpr float titleHeight  = 20.0f;   // height reserved for the title text
    static constexpr int   titleGap     = 2;       // gap between title bottom and content

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionPanel)
};

} // namespace gui
