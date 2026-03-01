/*
  ==============================================================================

    PluginEditor.cpp — UIToolkit Demo

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UItestAudioProcessorEditor::UItestAudioProcessorEditor (UItestAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      filterSection   (p.getAPVTS()),
      envelopeSection (p.getAPVTS())
{
    setLookAndFeel (&lnf);

    // Window size — wide enough to fit all demos
    setSize (720, 560);

    // --- 1. Standalone ArcKnobs (with SectionPanel background) ---
    addAndMakeVisible (knobPanel);
    addAndMakeVisible (gainKnob);
    addAndMakeVisible (panKnob);
    addAndMakeVisible (mixKnob);

    auto& apvts = audioProcessor.getAPVTS();
    gainAttach = std::make_unique<SliderAttach> (apvts, "gain", gainKnob.getSlider());
    panAttach  = std::make_unique<SliderAttach> (apvts, "pan",  panKnob.getSlider());
    mixAttach  = std::make_unique<SliderAttach> (apvts, "mix",  mixKnob.getSlider());

    // --- 2. SectionBase demos ---
    addAndMakeVisible (filterSection);
    addAndMakeVisible (envelopeSection);

    // --- 3. Controls panel (container for toggles + checkboxes) ---
    addAndMakeVisible (controlsPanel);

    // Add toggles & checkboxes as children of the panel
    controlsPanel.addAndMakeVisible (bypassSwitch);
    controlsPanel.addAndMakeVisible (hqModeSwitch);
    controlsPanel.addAndMakeVisible (oversampleCheck);
    controlsPanel.addAndMakeVisible (limiterCheck);

    bypassAttach = std::make_unique<ButtonAttach> (apvts, "bypass", bypassSwitch.getButton());
    hqModeAttach = std::make_unique<ButtonAttach> (apvts, "hqMode", hqModeSwitch.getButton());
    oversampleAttach = std::make_unique<ButtonAttach> (apvts, "oversample", oversampleCheck.getButton());
    limiterAttach    = std::make_unique<ButtonAttach> (apvts, "limiter",    limiterCheck.getButton());

    // --- Dropdowns as children of the panel ---
    controlsPanel.addAndMakeVisible (filterTypeDropdown);
    controlsPanel.addAndMakeVisible (waveformDropdown);

    // Items are populated from the AudioParameterChoice via attachment
    filterTypeDropdown.addItemList ({ "LPF", "HPF", "BPF", "Notch", "Peak" });
    waveformDropdown  .addItemList ({ "Sine", "Triangle", "Saw", "Square", "Noise" });

    // --- Text inputs as children of the panel ---
    controlsPanel.addAndMakeVisible (nameInput);
    controlsPanel.addAndMakeVisible (valueInput);

    filterTypeAttach = std::make_unique<ComboAttach> (apvts, "filterType", filterTypeDropdown.getComboBox());
    waveformAttach   = std::make_unique<ComboAttach> (apvts, "waveform",   waveformDropdown.getComboBox());
}

UItestAudioProcessorEditor::~UItestAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void UItestAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (gui::Colors::background);
}

void UItestAudioProcessorEditor::mouseDown (const juce::MouseEvent&)
{
    // Click on empty background → release keyboard focus from any TextInput
    unfocusAllComponents();
}

void UItestAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    // ===== Row 1: Standalone ArcKnobs inside a SectionPanel =====
    {
        auto knobPanelRow = area.removeFromTop (130);
        knobPanel.setBounds (knobPanelRow);

        auto content = knobPanel.getContentArea()
                           .translated (knobPanel.getX(), knobPanel.getY());
        const int knobW = content.getWidth() / 3;
        gainKnob.setBounds (content.removeFromLeft (knobW));
        panKnob .setBounds (content.removeFromLeft (knobW));
        mixKnob .setBounds (content);
    }

    area.removeFromTop (8);

    // ===== Row 2: Filter Section + Envelope Section (side by side) =====
    {
        auto sectionRow = area.removeFromTop (130);
        const int halfW = sectionRow.getWidth() / 2;
        filterSection  .setBounds (sectionRow.removeFromLeft (halfW).reduced (2, 0));
        envelopeSection.setBounds (sectionRow.reduced (2, 0));
    }

    area.removeFromTop (8);

    // ===== Row 3: Controls panel with Toggles + Checkboxes + Dropdowns + TextInputs =====
    {
        auto ctrlRow = area.removeFromTop (200);
        controlsPanel.setBounds (ctrlRow);

        // Use local coordinates since children belong to the panel
        auto content = controlsPanel.getContentArea();

        // Three columns: toggles | checkboxes | dropdowns & inputs
        const int colW = content.getWidth() / 3;
        auto leftCol   = content.removeFromLeft (colW).reduced (8, 4);
        auto midCol    = content.removeFromLeft (colW).reduced (8, 4);
        auto rightCol  = content.reduced (8, 4);

        constexpr int itemH = 32;

        // Left column: toggle switches
        bypassSwitch .setBounds (leftCol.removeFromTop (itemH));
        leftCol.removeFromTop (8);
        hqModeSwitch .setBounds (leftCol.removeFromTop (itemH));

        // Middle column: checkboxes
        oversampleCheck.setBounds (midCol.removeFromTop (itemH));
        midCol.removeFromTop (8);
        limiterCheck   .setBounds (midCol.removeFromTop (itemH));

        // Right column: dropdown selects + text inputs
        filterTypeDropdown.setBounds (rightCol.removeFromTop (itemH));
        rightCol.removeFromTop (8);
        waveformDropdown  .setBounds (rightCol.removeFromTop (itemH));
        rightCol.removeFromTop (8);
        nameInput  .setBounds (rightCol.removeFromTop (itemH));
        rightCol.removeFromTop (8);
        valueInput .setBounds (rightCol.removeFromTop (itemH));
    }
}
