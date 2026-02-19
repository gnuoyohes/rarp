#include "ADSRComponent.h"

ADSRComponent::ADSRComponent(juce::AudioProcessorValueTreeState& state)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        // slider
        sliders[i].setSliderStyle (juce::Slider::Rotary);
        sliders[i].setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 30);
        sliders[i].setTextBoxIsEditable (true);
        //sliders[i].textFromValueFunction = [] (double value) { return std::to_string(static_cast<int>(value * 1000)) + " ms"; };
        addAndMakeVisible (sliders[i]);

        // label
        labels[i].setFont (juce::Font (12.0f, juce::Font::bold));
        labels[i].setText (params[i], juce::dontSendNotification);
        labels[i].setColour (juce::Label::textColourId, juce::Colours::white);
        labels[i].setJustificationType (juce::Justification::centred);
        addAndMakeVisible (labels[i]);
        labels[i].toBack();

        // attachment
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, params[i], sliders[i]);
    }
}

//==============================================================================
void ADSRComponent::paint(juce::Graphics& g)
{
    return;
}

void ADSRComponent::resized()
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        const int width = getWidth();
        const int height = getHeight();

        const int sliderSize = 130;

        sliders[i].setBounds (i * width / 6.0f, 0, sliderSize, sliderSize);
        labels[i].setBounds (i * width / 6.0f, -13, sliderSize, sliderSize);
    }
}