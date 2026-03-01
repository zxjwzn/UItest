/*
  ==============================================================================
    TextInput.h - Dark-themed text input with animated focus & hover effects
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "Easings.h"
#include "HoverAnimatable.h"

namespace gui
{

/**
 * A dark-themed single-line text input with an optional left-side label.
 *
 * Wraps a juce::TextEditor with custom painting that matches the
 * UIToolkit dark theme. Features three layers of animation:
 *
 *   1. **Hover glow** — accent-colored border fades in/out via HoverAnimatable.
 *   2. **Focus-in** — border color transitions from panelBorder to accent,
 *      an accent underline slides in from center, and background brightens.
 *   3. **Focus-out** — all effects reverse smoothly.
 *
 * Animations are driven by juce::VBlankAnimatorUpdater for vsync-accurate
 * rendering. Mid-animation interruptions are handled seamlessly.
 *
 * Usage:
 * @code
 *   gui::TextInput nameInput{ "Name" };
 *   nameInput.setText("Hello");
 *   addAndMakeVisible(nameInput);
 *
 *   // Read value
 *   juce::String value = nameInput.getText();
 *
 *   // Listen for changes
 *   nameInput.onTextChange = [](const juce::String& text) { DBG(text); };
 * @endcode
 */
class TextInput : public juce::Component,
                  public HoverAnimatable<TextInput>
{
public:
    explicit TextInput(const juce::String& labelText = "",
                       const juce::String& placeholderText = "")
        : HoverAnimatable(this), label(labelText), focusAnimUpdater(this)
    {
        // Configure the internal TextEditor
        editor.setMultiLine(false);
        editor.setReturnKeyStartsNewLine(false);
        editor.setScrollbarsShown(false);
        editor.setCaretVisible(true);
        editor.setPopupMenuEnabled(true);

        // Dark-themed colors
        editor.setColour(juce::TextEditor::backgroundColourId,   juce::Colours::transparentBlack);
        editor.setColour(juce::TextEditor::textColourId,         Colors::textBright);
        editor.setColour(juce::TextEditor::highlightColourId,    Colors::accent.withAlpha(0.35f));
        editor.setColour(juce::TextEditor::highlightedTextColourId, Colors::textBright);
        editor.setColour(juce::TextEditor::outlineColourId,      juce::Colours::transparentBlack);
        editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        editor.setColour(juce::CaretComponent::caretColourId,    Colors::accent);

        editor.setFont(juce::FontOptions(13.0f));
        editor.setJustification(juce::Justification::centredLeft);

        if (placeholderText.isNotEmpty())
            editor.setTextToShowWhenEmpty(placeholderText, Colors::textDim);

        addAndMakeVisible(editor);
        addChildMouseListener(editor);

        // Focus animation triggers
        editor.onFocusGained = [this]() { startFocusAnimation(true);  };
        editor.onFocusLost   = [this]() { startFocusAnimation(false); };

        // Text-change callback forwarding
        editor.onTextChange = [this]()
        {
            if (onTextChange)
                onTextChange(editor.getText());
        };

        // Return-key callback forwarding
        editor.onReturnKeyPressed = [this]()
        {
            if (onReturnKey)
                onReturnKey();
        };
    }

    ~TextInput() override { removeChildMouseListener(editor); }

    // ── Paint ───────────────────────────────────────────────────

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float fp    = focusProgress;   // 0 = idle, 1 = focused
        const float hp    = getHoverProgress();

        // ── Optional label ──────────────────────────────────────
        if (label.isNotEmpty())
        {
            g.setColour(Colors::textDim);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(label, 0, 0, static_cast<int>(cachedLabelWidth + 6.0f),
                       getHeight(), juce::Justification::centredLeft);
        }

        // ── Editor area ─────────────────────────────────────────
        const auto edBounds = editor.getBounds().toFloat();
        constexpr float cornerSize = 5.0f;

        // Background — brightens slightly when focused
        const auto bgIdle   = Colors::knobBackground;
        const auto bgFocus  = Colors::knobBackground.brighter(0.08f);
        g.setColour(bgIdle.interpolatedWith(bgFocus, fp));
        g.fillRoundedRectangle(edBounds, cornerSize);

        // Border — transitions from panelBorder to accent on focus
        const auto borderIdle  = Colors::panelBorder;
        const auto borderFocus = Colors::accent;
        g.setColour(borderIdle.interpolatedWith(borderFocus, fp));
        g.drawRoundedRectangle(edBounds, cornerSize, 1.2f);

        // Hover glow border (only visible when not focused)
        const float hoverAlpha = hp * (1.0f - fp) * 0.6f;
        if (hoverAlpha > 0.001f)
        {
            g.setColour(Colors::accent.withAlpha(hoverAlpha));
            g.drawRoundedRectangle(edBounds.expanded(1.0f), cornerSize + 1.0f, 1.2f);
        }
    }

    // ── Layout ──────────────────────────────────────────────────

    void resized() override
    {
        constexpr int glowPad = 2;
        const float labelW = label.isNotEmpty() ? (cachedLabelWidth + 10.0f) : 0.0f;
        const int left = static_cast<int>(labelW) + glowPad;

        // Leave room for glow on all sides, inner padding for text
        editor.setBounds(left, glowPad,
                         getWidth() - left - glowPad,
                         getHeight() - glowPad * 2);
    }

    // ── Mouse forwarding ────────────────────────────────────────

    void mouseEnter(const juce::MouseEvent& e) override { HoverAnimatable::mouseEnter(e); }
    void mouseExit (const juce::MouseEvent& e) override { HoverAnimatable::mouseExit(e); }

    // ── Public API ──────────────────────────────────────────────

    /** Set the text content. */
    void setText(const juce::String& text, juce::NotificationType notification = juce::dontSendNotification)
    {
        editor.setText(text, notification == juce::sendNotification);
    }

    /** Get the current text content. */
    juce::String getText() const { return editor.getText(); }

    /** Set placeholder text shown when the editor is empty. */
    void setPlaceholder(const juce::String& placeholder)
    {
        editor.setTextToShowWhenEmpty(placeholder, Colors::textDim);
    }

    /** Set the maximum number of characters. 0 = unlimited. */
    void setMaxLength(int maxChars) { editor.setInputRestrictions(maxChars); }

    /** Restrict input to certain characters (e.g., "0123456789." for numbers). */
    void setAllowedCharacters(const juce::String& chars)
    {
        editor.setInputRestrictions(0, chars);
    }

    /** Access the underlying TextEditor for advanced configuration. */
    juce::TextEditor& getEditor() { return editor; }

    /** Callback fired when the text changes. */
    std::function<void(const juce::String&)> onTextChange;

    /** Callback fired when Return is pressed. */
    std::function<void()> onReturnKey;

private:
    // ── Animation ───────────────────────────────────────────────

    static constexpr double focusAnimDurationMs = 220.0;

    /** Launch focus-in or focus-out animation from current visual state. */
    void startFocusAnimation(bool focusing)
    {
        const float target = focusing ? 1.0f : 0.0f;

        // Remove the previous animator to avoid accumulation in the updater
        focusAnimUpdater.removeAnimator(focusAnimator);

        focusAnimator = juce::ValueAnimatorBuilder{}
            .withDurationMs(focusAnimDurationMs)
            .withEasing(focusing ? Easing::easeOutCubic() : Easing::easeInCubic())
            .withOnStartReturningValueChangedCallback([this, target]()
            {
                const float start = focusProgress;
                return [this, start, target](float progress)
                {
                    focusProgress = start + (target - start) * progress;
                    repaint();
                };
            })
            .build();

        focusAnimUpdater.addAnimator(focusAnimator);
        focusAnimator.start();
    }

    // ── Helpers ─────────────────────────────────────────────────

    // ── Wrapped TextEditor with focus callback hooks ────────────

    /** Lightweight TextEditor subclass that exposes focus events via lambdas. */
    class InternalEditor : public juce::TextEditor
    {
    public:
        using juce::TextEditor::TextEditor;

        std::function<void()> onFocusGained;
        std::function<void()> onFocusLost;

        void focusGained(FocusChangeType cause) override
        {
            juce::TextEditor::focusGained(cause);
            if (onFocusGained) onFocusGained();
        }

        void focusLost(FocusChangeType cause) override
        {
            juce::TextEditor::focusLost(cause);
            if (onFocusLost) onFocusLost();
        }

        /** Callback fired when Return is pressed (before focus is released). */
        std::function<void()> onReturnKeyPressed;

        bool keyPressed(const juce::KeyPress& key) override
        {
            if (key == juce::KeyPress::returnKey)
            {
                if (onReturnKeyPressed) onReturnKeyPressed();
                giveAwayKeyboardFocus();
                return true;
            }
            if (key == juce::KeyPress::escapeKey)
            {
                giveAwayKeyboardFocus();
                return true;
            }
            return juce::TextEditor::keyPressed(key);
        }
    };

    // ── Members ─────────────────────────────────────────────────

    InternalEditor editor;
    juce::String   label;

    /** Label width in pixels, computed once to avoid per-frame Font construction. */
    float cachedLabelWidth = [this]() {
        if (label.isEmpty()) return 0.0f;
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(11.0f)), label, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, -1, true).getWidth();
    }();

    juce::VBlankAnimatorUpdater focusAnimUpdater;
    juce::Animator              focusAnimator { juce::ValueAnimatorBuilder{}.build() };

    float focusProgress = 0.0f;   ///< 0 = idle, 1 = focused

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextInput)
};

} // namespace gui
