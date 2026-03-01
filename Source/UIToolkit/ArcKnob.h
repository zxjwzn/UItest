/*
  ==============================================================================
    ArcKnob.h - Custom arc-style rotary knob with value display
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
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
                public HoverAnimatable<ArcKnob>,
                private juce::Slider::Listener
{
public:
    ArcKnob(const juce::String& labelText, const juce::String& suffix = "")
        : HoverAnimatable(this), label(labelText), valueSuffix(suffix)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobArcActive);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobArcTrack);
        addAndMakeVisible(slider);

        // Custom painting — we override paint, slider is invisible but handles mouse
        slider.setAlpha(0.0f);
        slider.addListener(this);
        addChildMouseListener(slider);
    }

    ~ArcKnob() override
    {
        removeChildMouseListener(slider);
        slider.removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
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

        // Active arc
        const float proportion = static_cast<float>(
            slider.valueToProportionOfLength(slider.getValue()));
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
            juce::String valueStr;
            const double val = slider.getValue();

            if (std::abs(val) >= 100.0)
                valueStr = juce::String(static_cast<int>(val));
            else if (std::abs(val) >= 10.0)
                valueStr = juce::String(val, 1);
            else
                valueStr = juce::String(val, 2);

            valueStr += valueSuffix;

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
    void sliderValueChanged(juce::Slider*) override { repaint(); }

    juce::Slider slider;
    juce::String label;
    juce::String valueSuffix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcKnob)
};

} // namespace gui
