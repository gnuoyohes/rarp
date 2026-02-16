#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
                     //private juce::Slider::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
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

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    juce::ComboBox oscSelector;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> gainSliderAttachment;

    std::unique_ptr<SliderAttachment> attackSliderAttachment;
    std::unique_ptr<SliderAttachment> decaySliderAttachment;
    std::unique_ptr<SliderAttachment> sustainSliderAttachment;
    std::unique_ptr<SliderAttachment> releaseSliderAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelectorAttachment;

    juce::UndoManager& undoManager;
    juce::MidiKeyboardComponent midiKeyboard;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
