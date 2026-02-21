#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ArpeggiatorComponent : public juce::Component,
                             public juce::Button::Listener
{
public:
    ArpeggiatorComponent (juce::AudioProcessorValueTreeState& state);

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* button) override;

private:
    void createSliderAndAttachment (
        juce::AudioProcessorValueTreeState& state,
        juce::Slider& slider,
        int textBoxWidth,
        juce::Label& label,
        std::string labelText,
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
        std::string paramID);

    void createToggleButtonAndAttachment (
        juce::AudioProcessorValueTreeState& state,
        juce::ToggleButton& button,
        juce::Label& label,
        std::string labelText,
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment,
        std::string paramID);

    juce::Slider speedSlider;
    juce::Slider speedSyncSlider;
    juce::Slider randomizeSlider;
    juce::Slider densitySlider;
    juce::Slider widthSlider;

    juce::ToggleButton ascendingButton;
    juce::ToggleButton syncButton;

    juce::Label speedLabel;
    juce::Label speedSyncLabel;
    juce::Label randomizeLabel;
    juce::Label densityLabel;
    juce::Label widthLabel;
    juce::Label ascendingLabel;
    juce::Label syncLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedSyncSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randomizeSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densitySliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ascendingButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
};