#include "PluginEditor.h"
#include <string>

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), 
      processorRef (p),
      undoManager (processorRef.getUndoManager()),
      midiKeyboard (processorRef.getMidiKeyboardState(), juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{

    juce::AudioProcessorValueTreeState& state = processorRef.getState();

    adsrComponent = std::make_unique<ADSRComponent> (state);
    addAndMakeVisible (*adsrComponent);

    addAndMakeVisible (inspectButton);
    addAndMakeVisible (midiKeyboard);

    // Melatonin GUI inspector
    inspectButton.onClick = [&] {
        if (!inspector) {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
            inspector->setUndoManager (&undoManager);
        }

        inspector->setVisible (true);
    };

    setSize (800, 600);
    
    undoButton.onClick = [this] { undoManager.undo(); };
    addAndMakeVisible (undoButton);

    redoButton.onClick = [this] { undoManager.redo(); };
    addAndMakeVisible (redoButton);

    // Gain slider
    gainSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, true, 100, 50);
    gainSlider.setTextBoxIsEditable (true);
    addAndMakeVisible (gainSlider);

    gainLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    gainLabel.attachToComponent (&gainSlider, false);
    //gainLabel.setJustificationType (juce::Justification::bottom);
    addAndMakeVisible (gainLabel);

    // OSC ComboBox
    auto* oscParam = state.getParameter ("osc");
    oscSelector.addItemList (oscParam->getAllValueStrings(), 1);
    addAndMakeVisible (oscSelector);
    
    oscLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    oscLabel.setText ("Oscillator", juce::dontSendNotification);
    oscLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    oscLabel.attachToComponent (&oscSelector, false);
    addAndMakeVisible (oscLabel);

    addAndMakeVisible (processorRef.waveform);
    juce::LookAndFeel& defaultLookAndFeel = juce::LookAndFeel::getDefaultLookAndFeel();
    processorRef.waveform.setColours (defaultLookAndFeel.findColour (juce::Slider::backgroundColourId), defaultLookAndFeel.findColour (juce::Slider::thumbColourId));
    processorRef.waveform.setRepaintRate (30);
    processorRef.waveform.setBufferSize (512);

    // Speed slider
    speedSlider.setSliderStyle (juce::Slider::Rotary);
    speedSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 30);
    speedSlider.setTextBoxIsEditable (true);
    addAndMakeVisible (speedSlider);

    speedLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    speedLabel.setText ("Note Duration", juce::dontSendNotification);
    speedLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    speedLabel.attachToComponent (&speedSlider, false);
    addAndMakeVisible (speedLabel);

    // Initialize attachments
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, "gain", gainSlider);
    oscSelectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "osc", oscSelector);
    speedSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, "noteDur", speedSlider);
    // Use native title bar
    //auto* topLevel = juce::TopLevelWindow::getTopLevelWindow (0);
    //if (topLevel)
    //{
    //    topLevel->setUsingNativeTitleBar (true);
    //}
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    //auto area = getLocalBounds();
    //area.removeFromBottom (50);

    const int width = getWidth();
    const int height = getHeight();

    adsrComponent->setBounds (100, 100, width, height);

    inspectButton.setBounds (width - 200, 20, 80, 40);

    // MidiKeyboardComponent
    const int keyboardHeight = 100;
    midiKeyboard.setBounds (0, height - keyboardHeight, width, keyboardHeight);

    // Undo and redo buttons
    const int undoButtonSize = 30;
    undoButton.setBounds (width - (undoButtonSize * 2 + 30), 30, undoButtonSize, undoButtonSize);
    redoButton.setBounds (width - (undoButtonSize + 20), 30, undoButtonSize, undoButtonSize);

    gainSlider.setBounds (40, 50, 40, height / 4);

    oscSelector.setBounds (width / 4, 50, 100, 20);

    const int waveformX = 60;
    const int waveformY = 260;
    const int waveformWidth = 400;
    const int waveformHeight = 200;

    const int speedSliderSize = 130;

    processorRef.waveform.setBounds (waveformX, waveformY, waveformWidth, waveformHeight);

    speedSlider.setBounds (waveformX + waveformWidth + 30, waveformY + waveformHeight / 2 - speedSliderSize / 2, speedSliderSize, speedSliderSize);
    
}