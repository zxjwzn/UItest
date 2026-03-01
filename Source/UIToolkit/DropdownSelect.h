/*
  ==============================================================================
    DropdownSelect.h - Dark-themed dropdown (ComboBox) with optional label
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "HoverAnimatable.h"

namespace gui
{

/**
 * A custom dark-themed dropdown selector.
 *
 * Wraps a juce::ComboBox with custom painting that matches the
 * UIToolkit dark theme: rounded rectangle body, accent-colored
 * arrow indicator, and an optional left-side label.
 * Hover adds an accent-colored glow border around the combo box.
 *
 * Can be bound to an AudioParameterChoice via ComboBoxAttachment.
 *
 * Usage:
 * @code
 *   gui::DropdownSelect filterType{ "Type" };
 *   filterType.addItemList({ "LPF", "HPF", "BPF", "Notch" });
 *   addAndMakeVisible(filterType);
 *
 *   // APVTS binding
 *   auto attach = std::make_unique<
 *       juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
 *       apvts, "filterType", filterType.getComboBox());
 * @endcode
 */
class DropdownSelect : public juce::Component,
                       public HoverAnimatable<DropdownSelect>
{
public:
    explicit DropdownSelect(const juce::String& labelText = "")
        : HoverAnimatable(this), label(labelText)
    {
        comboBox.setJustificationType(juce::Justification::centredLeft);
        comboBox.setColour(juce::ComboBox::textColourId, Colors::textBright);

        // Popup menu colours
        comboBox.setColour(juce::PopupMenu::backgroundColourId, Colors::panelBackground);
        comboBox.setColour(juce::PopupMenu::textColourId, Colors::textBright);
        comboBox.setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::accent);
        comboBox.setColour(juce::PopupMenu::highlightedTextColourId, Colors::textBright);

        addAndMakeVisible(comboBox);
        addChildMouseListener(comboBox);
    }

    ~DropdownSelect() override { removeChildMouseListener(comboBox); }

    void paint(juce::Graphics& g) override
    {
        // Only draw the optional label; ComboBox draws itself via LookAndFeel
        if (label.isNotEmpty())
        {
            g.setColour(Colors::textDim);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(label, 0, 0, static_cast<int>(getLabelWidth() - 4.0f),
                       getHeight(), juce::Justification::centredLeft);
        }

        // Hover glow border around the combo box area
        const float hp = getHoverProgress();

        // After a popup closes the mouse may no longer be over us, but
        // mouseExit was never fired. Correct the stale hover here.
        if (hp > 0.001f && !isMouseOver(true))
        {
            startHoverAnimation(0.0f);
            return;   // will repaint via the fade-out animation
        }

        if (hp > 0.001f)
        {
            g.setColour(Colors::accent.withAlpha(hp * 0.6f));
            g.drawRoundedRectangle(comboBox.getBounds().toFloat().expanded(1.0f), 6.0f, 1.2f);
        }
    }

    void resized() override
    {
        constexpr int glowPad = 2;  // room for hover glow border on all sides
        const float labelW = label.isNotEmpty() ? getLabelWidth() : 0.0f;
        const int left = static_cast<int>(labelW) + glowPad;

        comboBox.setBounds(
            left, glowPad,
            getWidth() - left - glowPad,
            getHeight() - glowPad * 2);
    }

    void mouseEnter(const juce::MouseEvent& e) override { HoverAnimatable::mouseEnter(e); }
    void mouseExit (const juce::MouseEvent& e) override { HoverAnimatable::mouseExit(e); }

    /** Add a list of items (1-based IDs assigned automatically). */
    void addItemList(const juce::StringArray& items)
    {
        comboBox.addItemList(items, 1);
        if (comboBox.getSelectedId() == 0 && items.size() > 0)
            comboBox.setSelectedId(1, juce::dontSendNotification);
    }

    /** Access the underlying ComboBox (for ComboBoxAttachment). */
    juce::ComboBox& getComboBox() { return comboBox; }

    /** Get the currently selected item text. */
    juce::String getSelectedText() const { return comboBox.getText(); }

    /** Get the currently selected item index (0-based). */
    int getSelectedIndex() const { return comboBox.getSelectedItemIndex(); }

private:
    float getLabelWidth() const
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(11.0f)), label, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, -1, true).getWidth() + 10.0f;
    }

    juce::ComboBox comboBox;
    juce::String label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DropdownSelect)
};

} // namespace gui
