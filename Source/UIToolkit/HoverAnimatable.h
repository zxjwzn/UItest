/*
  ==============================================================================
    HoverAnimatable.h - CRTP mixin that adds smooth hover-in / hover-out
                        animation to any juce::Component subclass.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Easings.h"

namespace gui
{

/**
 * CRTP mixin that gives any Component a smoothly animated `hoverProgress`
 * value (0 = idle, 1 = fully hovered) driven by VBlankAnimatorUpdater.
 *
 * Because most UIToolkit components contain invisible child widgets
 * (ToggleButton, Slider, ComboBox) that cover the entire bounds,
 * standard mouseEnter/mouseExit on the parent alone won't work —
 * the parent immediately receives mouseExit when the cursor hits the child.
 *
 * Solution: the derived class must
 *   1. Forward mouseEnter / mouseExit to HoverAnimatable.
 *   2. Call `addChildMouseListener(child)` for each interactive child
 *      so that child-level mouseEnter/Exit is also forwarded.
 *
 * The mixin checks `isMouseOver(true)` (includes children) in mouseExit
 * so the hover only fades out when the cursor truly leaves the widget.
 *
 * Usage:
 * @code
 *   class MyWidget : public juce::Component,
 *                    public gui::HoverAnimatable<MyWidget>
 *   {
 *   public:
 *       MyWidget() : HoverAnimatable(this)
 *       {
 *           addAndMakeVisible(innerButton);
 *           addChildMouseListener(innerButton);  // forward mouse events
 *       }
 *       ~MyWidget() override { removeChildMouseListener(innerButton); }
 *
 *       void mouseEnter(const juce::MouseEvent& e) override { HoverAnimatable::mouseEnter(e); }
 *       void mouseExit (const juce::MouseEvent& e) override { HoverAnimatable::mouseExit(e); }
 *
 *       void paint(juce::Graphics& g) override
 *       {
 *           const float hp = getHoverProgress(); // 0..1
 *       }
 *   };
 * @endcode
 */
template <typename Derived>
class HoverAnimatable
{
public:
    /** Construct with a pointer to the owning Component (pass `this`). */
    explicit HoverAnimatable(juce::Component* owner)
        : hoverUpdater(owner)
    {
    }

    /** Current hover progress: 0 = idle, 1 = fully hovered. */
    float getHoverProgress() const noexcept { return hoverProgress; }

    // ── Mouse callbacks — should be called from Component ────────

    void mouseEnter(const juce::MouseEvent& /*e*/)
    {
        startHoverAnimation(1.0f);
    }

    void mouseExit(const juce::MouseEvent& /*e*/)
    {
        // Only fade out when the mouse has truly left the component
        // and all of its children (the invisible button / slider / etc.)
        if (!static_cast<Derived*>(this)->isMouseOver(true))
            startHoverAnimation(0.0f);
    }

protected:
    /** Register a child component so its mouseEnter / mouseExit events
     *  are forwarded to this mixin. Call in the constructor. */
    void addChildMouseListener(juce::Component& child)
    {
        child.addMouseListener(static_cast<Derived*>(this), false);
    }

    /** Unregister the child listener. Call in the destructor. */
    void removeChildMouseListener(juce::Component& child)
    {
        child.removeMouseListener(static_cast<Derived*>(this));
    }

    /** Override to change the hover animation duration (ms). Default 180. */
    static constexpr double hoverDurationMs = 180.0;

    /** Programmatically start the hover animation to a target value. */
    void startHoverAnimation(float target)
    {
        // Remove the previous animator to avoid accumulation in the updater
        hoverUpdater.removeAnimator(hoverAnimator);

        hoverAnimator = juce::ValueAnimatorBuilder{}
            .withDurationMs(hoverDurationMs)
            .withEasing(Easing::easeOutSine())
            .withOnStartReturningValueChangedCallback(
                [this, target]()
                {
                    const float start = hoverProgress;
                    return [this, start, target](float progress)
                    {
                        hoverProgress = start + (target - start) * progress;
                        static_cast<Derived*>(this)->repaint();
                    };
                })
            .build();

        hoverUpdater.addAnimator(hoverAnimator);
        hoverAnimator.start();
    }

private:

    juce::VBlankAnimatorUpdater hoverUpdater;
    juce::Animator              hoverAnimator{ juce::ValueAnimatorBuilder{}.build() };
    float                       hoverProgress = 0.0f;
};

} // namespace gui
