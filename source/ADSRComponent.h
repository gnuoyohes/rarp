#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ADSRComponent : public juce::Component
{
public:
    ADSRComponent (juce::AudioProcessorValueTreeState& state);

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    std::array<juce::Slider, 5> sliders;
    std::array<juce::Label, 5> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> attachments;
    std::array<std::string, 5> params { "attack", "decay", "sustain", "release", "expo" };

    //std::array<std::atomic<float>*, 5> adsrAtomic;
};