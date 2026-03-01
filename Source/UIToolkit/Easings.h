/*
  ==============================================================================
    Easings.h - Complete easing function library based on easings.net
    All cubic-bezier curves wrap juce::Easings::createCubicBezier().
    Elastic / Bounce use mathematical implementations.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <functional>

namespace gui
{

/**
 * A comprehensive set of easing functions following the easings.net standard.
 *
 * - Sine / Quad / Cubic / Quart / Quint / Expo / Circ / Back
 *   are implemented via juce::Easings::createCubicBezier() using the
 *   standard CSS cubic-bezier control-point approximations.
 *
 * - Elastic / Bounce require multi-segment or transcendental curves
 *   that cannot be represented as a single cubic Bezier, so they are
 *   implemented as mathematical functions.
 *
 * Every function returns std::function<float(float)> (input t ∈ [0,1])
 * to match the juce::ValueAnimatorBuilder::withEasing() signature.
 */
namespace Easing
{
    using EasingFn = std::function<float(float)>;

    // ── Helper: cubic-bezier shorthand ──────────────────────────
    inline EasingFn bezier(float x1, float y1, float x2, float y2)
    {
        return juce::Easings::createCubicBezier(x1, y1, x2, y2);
    }

    // ─────────────────────────────────────────────────────────────
    //  Sine
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInSine()    { return bezier(0.12f, 0.0f,  0.39f, 0.0f); }
    inline EasingFn easeOutSine()   { return bezier(0.61f, 1.0f,  0.88f, 1.0f); }
    inline EasingFn easeInOutSine() { return bezier(0.37f, 0.0f,  0.63f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Quad
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInQuad()    { return bezier(0.11f, 0.0f,  0.5f,  0.0f); }
    inline EasingFn easeOutQuad()   { return bezier(0.5f,  1.0f,  0.89f, 1.0f); }
    inline EasingFn easeInOutQuad() { return bezier(0.45f, 0.0f,  0.55f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Cubic
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInCubic()    { return bezier(0.32f, 0.0f,  0.67f, 0.0f); }
    inline EasingFn easeOutCubic()   { return bezier(0.33f, 1.0f,  0.68f, 1.0f); }
    inline EasingFn easeInOutCubic() { return bezier(0.65f, 0.0f,  0.35f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Quart
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInQuart()    { return bezier(0.5f,  0.0f,  0.75f, 0.0f); }
    inline EasingFn easeOutQuart()   { return bezier(0.25f, 1.0f,  0.5f,  1.0f); }
    inline EasingFn easeInOutQuart() { return bezier(0.76f, 0.0f,  0.24f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Quint
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInQuint()    { return bezier(0.64f, 0.0f,  0.78f, 0.0f); }
    inline EasingFn easeOutQuint()   { return bezier(0.22f, 1.0f,  0.36f, 1.0f); }
    inline EasingFn easeInOutQuint() { return bezier(0.83f, 0.0f,  0.17f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Expo
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInExpo()    { return bezier(0.7f,  0.0f,  0.84f, 0.0f); }
    inline EasingFn easeOutExpo()   { return bezier(0.16f, 1.0f,  0.3f,  1.0f); }
    inline EasingFn easeInOutExpo() { return bezier(0.87f, 0.0f,  0.13f, 1.0f); }

    // ─────────────────────────────────────────────────────────────
    //  Circ
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInCirc()    { return bezier(0.55f, 0.0f,  1.0f,  0.45f); }
    inline EasingFn easeOutCirc()   { return bezier(0.0f,  0.55f, 0.45f, 1.0f);  }
    inline EasingFn easeInOutCirc() { return bezier(0.85f, 0.0f,  0.15f, 1.0f);  }

    // ─────────────────────────────────────────────────────────────
    //  Back  (cubic-bezier with y outside [0,1] for overshoot)
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInBack()    { return bezier(0.36f, 0.0f,   0.66f, -0.56f); }
    inline EasingFn easeOutBack()   { return bezier(0.34f, 1.56f,  0.64f, 1.0f);   }
    inline EasingFn easeInOutBack() { return bezier(0.68f, -0.6f,  0.32f, 1.6f);   }

    // ─────────────────────────────────────────────────────────────
    //  Elastic  (mathematical — cannot be expressed as cubic Bezier)
    // ─────────────────────────────────────────────────────────────
    inline EasingFn easeInElastic()
    {
        return [](float t) -> float
        {
            if (t <= 0.0f) return 0.0f;
            if (t >= 1.0f) return 1.0f;
            constexpr float c = juce::MathConstants<float>::twoPi / 3.0f;
            return -std::pow(2.0f, 10.0f * t - 10.0f)
                   * std::sin((10.0f * t - 10.75f) * c);
        };
    }

    inline EasingFn easeOutElastic()
    {
        return [](float t) -> float
        {
            if (t <= 0.0f) return 0.0f;
            if (t >= 1.0f) return 1.0f;
            constexpr float c = juce::MathConstants<float>::twoPi / 3.0f;
            return std::pow(2.0f, -10.0f * t)
                   * std::sin((10.0f * t - 0.75f) * c) + 1.0f;
        };
    }

    inline EasingFn easeInOutElastic()
    {
        return [](float t) -> float
        {
            if (t <= 0.0f) return 0.0f;
            if (t >= 1.0f) return 1.0f;
            constexpr float c = juce::MathConstants<float>::twoPi / 4.5f;
            if (t < 0.5f)
                return -(std::pow(2.0f,  20.0f * t - 10.0f)
                         * std::sin((20.0f * t - 11.125f) * c)) * 0.5f;
            return (std::pow(2.0f, -20.0f * t + 10.0f)
                    * std::sin((20.0f * t - 11.125f) * c)) * 0.5f + 1.0f;
        };
    }

    // ─────────────────────────────────────────────────────────────
    //  Bounce  (mathematical — cannot be expressed as cubic Bezier)
    // ─────────────────────────────────────────────────────────────
    namespace detail
    {
        inline float bounceOut(float t)
        {
            constexpr float n1 = 7.5625f;
            constexpr float d1 = 2.75f;

            if (t < 1.0f / d1)
                return n1 * t * t;

            if (t < 2.0f / d1)
            {
                t -= 1.5f / d1;
                return n1 * t * t + 0.75f;
            }

            if (t < 2.5f / d1)
            {
                t -= 2.25f / d1;
                return n1 * t * t + 0.9375f;
            }

            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    } // namespace detail

    inline EasingFn easeInBounce()
    {
        return [](float t) -> float
        {
            return 1.0f - detail::bounceOut(1.0f - t);
        };
    }

    inline EasingFn easeOutBounce()
    {
        return [](float t) -> float
        {
            return detail::bounceOut(t);
        };
    }

    inline EasingFn easeInOutBounce()
    {
        return [](float t) -> float
        {
            if (t < 0.5f)
                return (1.0f - detail::bounceOut(1.0f - 2.0f * t)) * 0.5f;
            return (1.0f + detail::bounceOut(2.0f * t - 1.0f)) * 0.5f;
        };
    }

} // namespace Easing
} // namespace gui
