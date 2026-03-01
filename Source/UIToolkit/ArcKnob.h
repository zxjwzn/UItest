/*
  ==============================================================================
    ArcKnob.h - Custom arc-style rotary knob with value display
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
 * A custom rotary knob that draws:
 *   - A dark circle background
 *   - A 270° arc track (dimmed, lights up on hover)
 *   - A colored arc for the current value
 *   - Current value text at center
 *   - Parameter label below
 *   - Thumb dot that grows on hover
 */
class ArcKnob : public juce::Component,
                public HoverAnimatable<ArcKnob>
{
public:
    ArcKnob(const juce::String& labelText, const juce::String& suffix = "")
        : HoverAnimatable(this), label(labelText), valueSuffix(suffix),
          introAnimUpdater(this)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobArcActive);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobArcTrack);
        addAndMakeVisible(slider);

        // Custom painting — we override paint, slider is invisible but handles mouse
        slider.setAlpha(0.0f);
        slider.onValueChange = [this]() { repaint(); };
        addChildMouseListener(slider);
    }

    ~ArcKnob() override
    {
        removeChildMouseListener(slider);
    }

    void paint(juce::Graphics& g) override
    {
        // Trigger intro animation on first paint (component is on-screen,
        // APVTS values are restored, VBlank peer is available).
        if (!introPlayed)
        {
            introPlayed = true;
            startIntroAnimation();
        }

        const auto bounds = getLocalBounds().toFloat();
        const float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 18.0f);
        const float radius = knobSize * 0.42f;
        const float arcThickness = 3.5f;
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getY() + knobSize * 0.5f;

        constexpr float startAngle = juce::MathConstants<float>::pi * 1.25f;
        constexpr float endAngle = juce::MathConstants<float>::pi * 2.75f;

        // Knob background circle
        g.setColour(Colors::knobBackground);
        g.fillEllipse(centreX - radius - 4.0f, centreY - radius - 4.0f,
                       (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);

        // Background arc track — brightens on hover
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            startAngle, endAngle, true);
        const float hp = getHoverProgress();
        const auto trackColour = Colors::knobArcTrack.interpolatedWith(
            Colors::accent.withAlpha(0.25f), hp);
        g.setColour(trackColour);
        g.strokePath(bgArc, juce::PathStrokeType(arcThickness,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active arc — scaled by introProgress for startup animation
        const float rawProportion = static_cast<float>(
            slider.valueToProportionOfLength(slider.getValue()));
        const float proportion = rawProportion * introProgress;
        const float currentAngle = startAngle + proportion * (endAngle - startAngle);

        if (proportion > 0.001f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                                    startAngle, currentAngle, true);

            juce::ColourGradient gradient(Colors::accentDark, centreX - radius, centreY,
                                          Colors::accent, centreX + radius, centreY, false);
            g.setGradientFill(gradient);
            g.strokePath(activeArc, juce::PathStrokeType(arcThickness,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Thumb dot — grows on hover (3 → 4.5)
        {
            const float thumbRadius = 3.0f + hp * 1.5f;
            const float thumbX = centreX + radius * std::sin(currentAngle);
            const float thumbY = centreY - radius * std::cos(currentAngle);
            g.setColour(Colors::knobThumb);
            g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius,
                          thumbRadius * 2.0f, thumbRadius * 2.0f);
        }

        // Value text at center
        {
            juce::String valueStr = slider.getTextFromValue(slider.getValue()) + valueSuffix;

            g.setColour(Colors::textBright);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(valueStr,
                       static_cast<int>(centreX - radius),
                       static_cast<int>(centreY - 7),
                       static_cast<int>(radius * 2.0f), 14,
                       juce::Justification::centred);
        }

        // Label below knob
        {
            g.setColour(Colors::textDim);
            g.setFont(juce::FontOptions(10.0f));
            g.drawText(label,
                       static_cast<int>(bounds.getX()),
                       static_cast<int>(centreY + radius + 6.0f),
                       static_cast<int>(bounds.getWidth()), 14,
                       juce::Justification::centred);
        }
    }

    void resized() override
    {
        slider.setBounds(getLocalBounds());
    }

    void mouseEnter(const juce::MouseEvent& e) override { HoverAnimatable::mouseEnter(e); }
    void mouseExit (const juce::MouseEvent& e) override { HoverAnimatable::mouseExit(e); }

    bool hitTest(int x, int y) override
    {
        const auto b = getLocalBounds().toFloat();
        const float knobSize = juce::jmin(b.getWidth(), b.getHeight() - 18.0f);
        const float outerR   = knobSize * 0.42f + 6.0f;   // bg circle + tolerance
        const float cx       = b.getCentreX();
        const float cy       = b.getY() + knobSize * 0.5f;

        const float dx = static_cast<float>(x) - cx;
        const float dy = static_cast<float>(y) - cy;
        if (dx * dx + dy * dy <= outerR * outerR)
            return true;

        // Include the label area below the knob
        const float labelTop = cy + knobSize * 0.42f + 6.0f;
        return static_cast<float>(y) >= labelTop
            && static_cast<float>(y) <= labelTop + 14.0f;
    }

    juce::Slider& getSlider() { return slider; }

private:
    // ── Intro animation ─────────────────────────────────────────

    /** Each knob gets a random duration between 400–800 ms so they don't
     *  all land at the same time — feels more organic. */
    double introAnimDurationMs = 300.0 + juce::Random::getSystemRandom().nextDouble() * 700.0;

    /** Animate introProgress from 0 → 1 so the arc sweeps from zero to the
     *  actual slider value. Called once on first paint. */
    void startIntroAnimation()
    {
        introAnimUpdater.removeAnimator(introAnimator);

        introAnimator = juce::ValueAnimatorBuilder{}
            .withDurationMs(introAnimDurationMs)
            .withEasing(Easing::easeOutQuart())
            .withValueChangedCallback([this](float progress)
            {
                introProgress = progress;
                repaint();
            })
            .build();

        introAnimUpdater.addAnimator(introAnimator);
        introAnimator.start();
    }

    // ── Members ─────────────────────────────────────────────────

    juce::Slider slider;
    juce::String label;
    juce::String valueSuffix;   ///< Kept for reference; display now uses slider.getTextFromValue()

    juce::VBlankAnimatorUpdater introAnimUpdater;
    juce::Animator              introAnimator { juce::ValueAnimatorBuilder{}.build() };
    float introProgress  = 0.0f;   ///< 0 = start, 1 = fully revealed
    bool  introPlayed    = false;  ///< Ensures the animation runs only once

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcKnob)
};

} // namespace gui
