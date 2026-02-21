#include "ArpeggiatorComponent.h"

ArpeggiatorComponent::ArpeggiatorComponent (juce::AudioProcessorValueTreeState& state)
{
    createSliderAndAttachment (state, speedSlider, 60, speedLabel, "Note Duration", speedSliderAttachment, "noteDur");
    createSliderAndAttachment (state, speedSyncSlider, 70, speedSyncLabel, "Note Duration", speedSyncSliderAttachment, "noteDurSync");
    createSliderAndAttachment (state, randomizeSlider, 50, randomizeLabel, "Randomize", randomizeSliderAttachment, "randomize");
    createSliderAndAttachment (state, densitySlider, 50, densityLabel, "Density", densitySliderAttachment, "density");

    createToggleButtonAndAttachment (state, ascendingButton, ascendingLabel, "Ascending", ascendingButtonAttachment, "ascending");
    createToggleButtonAndAttachment (state, syncButton, syncLabel, "Sync to BPM", syncButtonAttachment, "sync");
    
    // Add button listener
    syncButton.addListener (this);

    buttonClicked (dynamic_cast<juce::Button*> (&syncButton));

    //float sync = state.getRawParameterValue ("sync")->load();
    //if (syncButton.getToggleState)
    //    speedSlider.setVisible (false);
    //else
    //    speedSyncSlider.setVisible (false);

    // Width slider
    widthSlider.setSliderStyle (juce::Slider::LinearBar);
    widthSlider.setTextBoxStyle (juce::Slider::TextBoxRight, true, 100, 50);
    widthSlider.setTextBoxIsEditable (true);
    addAndMakeVisible (widthSlider);

    widthLabel.setFont (juce::Font (14.0f, juce::Font::bold));
    widthLabel.setText ("Randomize Pan Width", juce::dontSendNotification);
    widthLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    widthLabel.attachToComponent (&widthSlider, false);
    widthLabel.setJustificationType (juce::Justification::top);
    addAndMakeVisible (widthLabel);

    widthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, "width", widthSlider);
}

//==============================================================================
void ArpeggiatorComponent::paint (juce::Graphics& g)
{
    return;
}

void ArpeggiatorComponent::resized()
{
    const int sliderYOffset = 20;
    const int sliderSize = 130;

    speedSlider.setBounds (10, sliderYOffset, sliderSize, sliderSize);
    speedLabel.setBounds (speedSlider.getX(), speedSlider.getY() - 15, sliderSize, sliderSize);

    speedSyncSlider.setBounds (10, sliderYOffset, sliderSize, sliderSize);
    speedSyncLabel.setBounds (speedSlider.getX(), speedSlider.getY() - 15, sliderSize, sliderSize);

    randomizeSlider.setBounds (speedSlider.getX() + sliderSize + 10, sliderYOffset, sliderSize, sliderSize);
    randomizeLabel.setBounds (randomizeSlider.getX(), randomizeSlider.getY() - 15, sliderSize, sliderSize);

    densitySlider.setBounds (randomizeSlider.getX() + sliderSize + 10, sliderYOffset, sliderSize, sliderSize);
    densityLabel.setBounds (densitySlider.getX(), densitySlider.getY() - 15, sliderSize, sliderSize);

    ascendingButton.setBounds (45, speedSlider.getY() + sliderSize + 40, 100, 40);
    syncButton.setBounds (ascendingButton.getX() + 100, ascendingButton.getY(), 100, 40);

    const int widthSliderSize = 150;
    widthSlider.setBounds (getWidth() - widthSliderSize - 20, ascendingButton.getY(), widthSliderSize, 40);
}

void ArpeggiatorComponent::buttonClicked (juce::Button* button)
{
    if (button == &syncButton)
    {
        if (button->getToggleState())
        {
            speedSlider.setVisible (false);
            speedSyncSlider.setVisible (true);
        }
        else
        {
            speedSlider.setVisible (true);
            speedSyncSlider.setVisible (false);
        }
    }
}

void ArpeggiatorComponent::createSliderAndAttachment (
    juce::AudioProcessorValueTreeState& state,
    juce::Slider& slider,
    int textBoxWidth,
    juce::Label& label,
    std::string labelText,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
    std::string paramID)
{
    slider.setSliderStyle (juce::Slider::Rotary);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, textBoxWidth, 30);
    slider.setTextBoxIsEditable (true);
    addAndMakeVisible (slider);

    label.setFont (juce::Font (14.0f, juce::Font::bold));
    label.setText (labelText, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType (juce::Justification::centredTop);
    label.attachToComponent (&slider, false);
    addAndMakeVisible (label);
    label.toBack();
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramID, slider);
}

void ArpeggiatorComponent::createToggleButtonAndAttachment(
    juce::AudioProcessorValueTreeState& state,
    juce::ToggleButton& button,
    juce::Label& label,
    std::string labelText,
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment,
    std::string paramID)
{
    addAndMakeVisible (button);
    label.setFont (juce::Font (14.0f, juce::Font::bold));
    label.setText (labelText, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.attachToComponent (&button, false);
    addAndMakeVisible (label);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, paramID, button);
}