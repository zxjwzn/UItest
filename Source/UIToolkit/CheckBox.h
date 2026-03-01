/*
  ==============================================================================
    CheckBox.h - Dark-themed checkbox with label
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
 * A custom checkbox that draws a rounded-square box with
 * an accent-colored check mark and an optional label.
 * Hover adds an accent-colored glow border around the box.
 *
 * Wraps a juce::ToggleButton and can be bound to a boolean
 * AudioProcessorValueTreeState parameter via ButtonAttachment.
 */
class CheckBox : public juce::Component,
                 public HoverAnimatable<CheckBox>
{
public:
    explicit CheckBox(const juce::String& labelText = "")
        : HoverAnimatable(this), label(labelText), checkAnimUpdater(this)
    {
        button.setClickingTogglesState(true);
        button.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
        button.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(button);

        // Custom painting — button is invisible but handles mouse / state
        button.setAlpha(0.0f);
        button.onStateChange = [this]() { startCheckAnimation(); };
        addChildMouseListener(button);
    }

    ~CheckBox() override { removeChildMouseListener(button); }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float cp = checkProgress;   // 0 = unchecked, 1 = checked

        // Box dimensions
        constexpr float boxSize = 16.0f;
        const float boxY = (bounds.getHeight() - boxSize) * 0.5f;
        const float boxX = bounds.getX() + 3.0f;
        const auto boxBounds = juce::Rectangle<float>(boxX, boxY, boxSize, boxSize);

        // Draw box background — interpolate between knobBackground and accent tint
        const auto bgOff = Colors::knobBackground;
        const auto bgOn  = Colors::accent.withAlpha(0.2f);
        g.setColour(bgOff.interpolatedWith(bgOn, cp));
        g.fillRoundedRectangle(boxBounds, 3.0f);

        // Draw border — interpolate between panelBorder and accent
        g.setColour(Colors::panelBorder.interpolatedWith(Colors::accent, cp));
        g.drawRoundedRectangle(boxBounds, 3.0f, 1.2f);

        // Hover glow border
        const float hp = getHoverProgress();
        if (hp > 0.001f)
        {
            g.setColour(Colors::accent.withAlpha(hp * 0.6f));
            g.drawRoundedRectangle(boxBounds.expanded(1.5f), 4.0f, 1.2f);
        }

        // Draw check mark with progressive stroke animation
        if (cp > 0.001f)
        {
            // Three key points of the ✓ shape
            const juce::Point<float> p0 { boxX + 3.5f,              boxY + boxSize * 0.50f };
            const juce::Point<float> p1 { boxX + boxSize * 0.40f,   boxY + boxSize * 0.72f };
            const juce::Point<float> p2 { boxX + boxSize - 3.5f,    boxY + boxSize * 0.30f };

            // Segment lengths
            const float seg1 = p0.getDistanceFrom(p1);
            const float seg2 = p1.getDistanceFrom(p2);
            const float totalLen = seg1 + seg2;
            const float drawnLen = cp * totalLen;

            juce::Path checkPath;
            checkPath.startNewSubPath(p0);

            if (drawnLen <= seg1)
            {
                // Still drawing segment 1 (p0 → p1)
                const float t = drawnLen / seg1;
                checkPath.lineTo(p0 + (p1 - p0) * t);
            }
            else
            {
                // Full segment 1, partial segment 2 (p1 → p2)
                checkPath.lineTo(p1);
                const float t = (drawnLen - seg1) / seg2;
                checkPath.lineTo(p1 + (p2 - p1) * t);
            }

            g.setColour(Colors::accent);
            g.strokePath(checkPath, juce::PathStrokeType(2.0f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Label text
        if (label.isNotEmpty())
        {
            const float labelX = boxX + boxSize + 8.0f;
            g.setColour(Colors::textBright);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(label,
                       static_cast<int>(labelX),
                       0,
                       static_cast<int>(bounds.getWidth() - labelX),
                       static_cast<int>(bounds.getHeight()),
                       juce::Justification::centredLeft);
        }
    }

    void resized() override
    {
        button.setBounds(getLocalBounds());
    }

    void mouseEnter(const juce::MouseEvent& e) override { HoverAnimatable::mouseEnter(e); }
    void mouseExit (const juce::MouseEvent& e) override { HoverAnimatable::mouseExit(e); }

    bool hitTest(int x, int y) override
    {
        if (y < 0 || y >= getHeight()) return false;
        constexpr float boxSize = 16.0f;
        constexpr float padLeft = 3.0f;
        float contentRight = padLeft + boxSize;
        if (label.isNotEmpty())
            contentRight += 8.0f + juce::Font(juce::FontOptions(11.0f)).getStringWidthFloat(label);
        return static_cast<float>(x) >= 0.0f && static_cast<float>(x) <= contentRight;
    }

    /** Access the underlying ToggleButton (for ButtonAttachment). */
    juce::ToggleButton& getButton() { return button; }

    /** Query the current checked state. */
    bool isChecked() const { return button.getToggleState(); }

    /** Programmatically set the state. */
    void setChecked(bool on, juce::NotificationType notification = juce::sendNotification)
    {
        button.setToggleState(on, notification);
        repaint();
    }

private:
    // ── Animation ───────────────────────────────────────────────

    static constexpr double checkAnimDurationMs = 200.0;

    /** Launch (or re-launch) the check/uncheck animation. */
    void startCheckAnimation()
    {
        const bool checked = button.getToggleState();
        if (checked == lastCheckedState)
            return;

        lastCheckedState = checked;
        const float target = checked ? 1.0f : 0.0f;

        checkAnimator = juce::ValueAnimatorBuilder{}
            .withDurationMs(checkAnimDurationMs)
            .withEasing(Easing::easeOutCubic())
            .withOnStartReturningValueChangedCallback([this, target]()
            {
                const float start = checkProgress;
                return [this, start, target](float progress)
                {
                    checkProgress = start + (target - start) * progress;
                    repaint();
                };
            })
            .build();

        checkAnimUpdater.addAnimator(checkAnimator);
        checkAnimator.start();
    }

    // ── Members ─────────────────────────────────────────────────

    juce::ToggleButton button;
    juce::String label;

    juce::VBlankAnimatorUpdater checkAnimUpdater;
    juce::Animator              checkAnimator { juce::ValueAnimatorBuilder{}.build() };

    float checkProgress      = 0.0f;   ///< 0 = unchecked, 1 = checked
    bool  lastCheckedState   = false;   ///< Tracks state to detect real changes

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CheckBox)
};

} // namespace gui
