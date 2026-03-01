/*
  ==============================================================================
    ToggleSwitch.h - iOS-style toggle switch with label
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
 * An iOS/Material-style toggle switch with an optional label on the right.
 *
 * Wraps a juce::ToggleButton but draws a custom pill-shaped track
 * with an animated circular thumb. Smooth position and color transitions
 * are driven by juce::VBlankAnimatorUpdater (synced to display refresh)
 * with ease-out easing. Supports mid-animation interruption gracefully.
 * Hover adds an accent-colored glow border around the track.
 *
 * Can be bound to a boolean AudioProcessorValueTreeState parameter
 * via ButtonAttachment.
 */
class ToggleSwitch : public juce::Component,
                     public HoverAnimatable<ToggleSwitch>
{
public:
    explicit ToggleSwitch(const juce::String& labelText = "")
        : HoverAnimatable(this), label(labelText), animatorUpdater(this)
    {
        button.setClickingTogglesState(true);
        button.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
        button.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(button);

        // Custom painting — button is invisible but handles mouse / state
        button.setAlpha(0.0f);
        addChildMouseListener(button);

        // React to every state change (user click *and* APVTS-driven changes)
        button.onStateChange = [this]() { startAnimation(); };

        // Sync initial visual state without animation when the button is
        // already toggled on (e.g. restored from APVTS saved state).
        syncInitialState();
    }

    ~ToggleSwitch() override { removeChildMouseListener(button); }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();

        // Track dimensions
        constexpr float trackHeight = 18.0f;
        constexpr float trackWidth  = 34.0f;
        const float trackY = (bounds.getHeight() - trackHeight) * 0.5f;
        const float trackX = bounds.getX() + 2.0f;

        const auto trackBounds = juce::Rectangle<float>(trackX, trackY, trackWidth, trackHeight);
        const float trackRadius = trackHeight * 0.5f;

        // Draw track — color interpolated between off and on states
        const auto trackColour = Colors::knobArcTrack.interpolatedWith(
            Colors::accent.withAlpha(0.5f), animProgress);
        g.setColour(trackColour);
        g.fillRoundedRectangle(trackBounds, trackRadius);

        // Hover glow border
        const float hp = getHoverProgress();
        if (hp > 0.001f)
        {
            g.setColour(Colors::accent.withAlpha(hp * 0.6f));
            g.drawRoundedRectangle(trackBounds.expanded(1.0f), trackRadius + 1.0f, 1.2f);
        }

        // Draw thumb — position interpolated between off and on
        constexpr float thumbDiameter = 14.0f;
        const float thumbY = trackY + (trackHeight - thumbDiameter) * 0.5f;
        const float thumbXOff = trackX + 2.0f;
        const float thumbXOn  = trackX + trackWidth - thumbDiameter - 2.0f;
        const float thumbX = thumbXOff + animProgress * (thumbXOn - thumbXOff);

        // Thumb color interpolated between dim and accent
        const auto thumbColour = Colors::textDim.interpolatedWith(Colors::accent, animProgress);
        g.setColour(thumbColour);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

        // Subtle border on thumb
        g.setColour(Colors::knobThumb.withAlpha(0.3f));
        g.drawEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter, 0.8f);

        // Label text
        if (label.isNotEmpty())
        {
            const float labelX = trackX + trackWidth + 8.0f;
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
        constexpr float trackWidth = 34.0f;
        constexpr float padLeft = 2.0f;
        float contentRight = padLeft + trackWidth;
        if (label.isNotEmpty())
            contentRight += 8.0f + cachedLabelWidth;
        return static_cast<float>(x) >= 0.0f && static_cast<float>(x) <= contentRight;
    }

    /** Access the underlying ToggleButton (for ButtonAttachment). */
    juce::ToggleButton& getButton() { return button; }

    /** Query the current on/off state. */
    bool isOn() const { return button.getToggleState(); }

    /** Programmatically set the state (animation will play automatically). */
    void setOn(bool on, juce::NotificationType notification = juce::sendNotification)
    {
        button.setToggleState(on, notification);
    }

private:
    // ── Animation ───────────────────────────────────────────────

    static constexpr double animDurationMs = 250.0;

    /** Launch (or re-launch) the toggle animation from the current visual position. */
    void startAnimation()
    {
        const bool on = button.getToggleState();
        if (on == lastToggleState)
            return;

        lastToggleState = on;
        const float target = on ? 1.0f : 0.0f;

        // Build a new animator that interpolates from the current visual
        // position to the target, so mid-animation reversals are seamless.
        // Remove the previous animator to avoid accumulation in the updater
        animatorUpdater.removeAnimator(animator);

        animator = juce::ValueAnimatorBuilder{}
            .withDurationMs(animDurationMs)
            .withEasing(Easing::easeOutCubic())
            .withOnStartReturningValueChangedCallback([this, target]()
            {
                const float start = animProgress;
                return [this, start, target](float progress)
                {
                    animProgress = start + (target - start) * progress;
                    repaint();
                };
            })
            .build();

        animatorUpdater.addAnimator(animator);
        animator.start();
    }

    // ── Members ─────────────────────────────────────────────────

    juce::ToggleButton button;
    juce::String       label;

    /** Label width in pixels, computed once to avoid per-frame Font construction. */
    float cachedLabelWidth = [this]() {
        if (label.isEmpty()) return 0.0f;
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(11.0f)), label, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, -1, true).getWidth();
    }();

    juce::VBlankAnimatorUpdater animatorUpdater;
    juce::Animator              animator { juce::ValueAnimatorBuilder{}.build() };

    float animProgress    = 0.0f;    ///< Current visual position: 0 = off, 1 = on
    bool  lastToggleState = false;   ///< Tracks toggle state to detect real changes

    /** Jump to the correct visual state without animation (called once in ctor). */
    void syncInitialState()
    {
        const bool on = button.getToggleState();
        animProgress    = on ? 1.0f : 0.0f;
        lastToggleState = on;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitch)
};

} // namespace gui
