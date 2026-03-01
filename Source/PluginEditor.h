/*
  ==============================================================================

    PluginEditor.h — UIToolkit Demo: showcases every component

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UIToolkit/UIToolkit.h"

//==============================================================================
/**
 * Demo Section built with SectionBase (Filter).
 * Shows: SectionPanel background + KnobStrip with APVTS binding.
 */
class DemoFilterSection : public gui::SectionBase
{
public:
    DemoFilterSection (juce::AudioProcessorValueTreeState& apvts)
        : SectionBase ("FILTER", apvts,
          {
              { "Cutoff", " Hz", "filterCutoff" },
              { "Reso",   " %",  "filterReso"   },
              { "Drive",  " %",  "filterDrive"   }
          })
    {}
};

/**
 * Demo Section built with SectionBase (Envelope).
 * 4 knobs filling the whole content area (knobRowHeight = 0).
 */
class DemoEnvelopeSection : public gui::SectionBase
{
public:
    DemoEnvelopeSection (juce::AudioProcessorValueTreeState& apvts)
        : SectionBase ("ENVELOPE", apvts,
          {
              { "Attack",  " ms", "envAttack"  },
              { "Decay",   " ms", "envDecay"   },
              { "Sustain", " %",  "envSustain" },
              { "Release", " ms", "envRelease" }
          }, 0)              // knobs fill entire area
    {}
};

//==============================================================================
class UItestAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    UItestAudioProcessorEditor (UItestAudioProcessor&);
    ~UItestAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    UItestAudioProcessor& audioProcessor;

    // --- LookAndFeel ---
    gui::CustomLookAndFeel lnf;

    // --- 1. Standalone ArcKnobs (wrapped in SectionPanel) ---
    gui::SectionPanel knobPanel { "ARC KNOB" };

    gui::ArcKnob gainKnob  { "Gain",  " dB" };
    gui::ArcKnob panKnob   { "Pan",   ""    };
    gui::ArcKnob mixKnob   { "Mix",   " %"  };

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttach> gainAttach, panAttach, mixAttach;

    // --- 2. SectionBase demos (KnobStrip + SectionPanel) ---
    DemoFilterSection   filterSection;
    DemoEnvelopeSection envelopeSection;

    // --- 3. Toggle switches ---
    gui::ToggleSwitch bypassSwitch   { "Bypass" };
    gui::ToggleSwitch hqModeSwitch   { "HQ Mode" };

    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttach> bypassAttach, hqModeAttach;

    // --- 4. Checkboxes ---
    gui::CheckBox oversampleCheck { "2x Oversampling" };
    gui::CheckBox limiterCheck    { "Limiter" };

    std::unique_ptr<ButtonAttach> oversampleAttach, limiterAttach;

    // --- 5. Dropdown selects ---
    gui::DropdownSelect filterTypeDropdown { "Filter" };
    gui::DropdownSelect waveformDropdown   { "Wave" };

    using ComboAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboAttach> filterTypeAttach, waveformAttach;

    // --- 6. Text inputs ---
    gui::TextInput nameInput   { "Name",  "Enter name..."  };
    gui::TextInput valueInput  { "Value", "0.0" };

    // --- 7. Standalone SectionPanel (container demo) ---
    gui::SectionPanel controlsPanel { "CONTROLS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UItestAudioProcessorEditor)
};
