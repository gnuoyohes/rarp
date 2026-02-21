#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include "ADSRComponent.h"
#include "ArpeggiatorComponent.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor& p);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    std::unique_ptr<melatonin::Inspector> inspector;

    juce::TextButton inspectButton { "Inspect the UI" };
    juce::TextButton undoButton { juce::String::fromUTF8 ("↶") };
    juce::TextButton redoButton { juce::String::fromUTF8 ("↷") };

    juce::Slider gainSlider;
    juce::Label gainLabel;

    std::unique_ptr<ADSRComponent> adsrComponent;

    std::unique_ptr<ArpeggiatorComponent> arpeggiatorComponent;

    juce::ComboBox oscSelector;
    juce::Label oscLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelectorAttachment;

    juce::UndoManager& undoManager;
    juce::MidiKeyboardComponent midiKeyboard;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
