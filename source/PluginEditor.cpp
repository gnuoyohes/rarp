#include "PluginEditor.h"
#include <windows.h>
#include <string>

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), 
      processorRef (p),
      undoManager (processorRef.getUndoManager()),
      midiKeyboard (processorRef.getMidiKeyboardState(), juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    // Initialize attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    gainSliderAttachment = std::make_unique<SliderAttachment> (processorRef.getState(), "gain", gainSlider);
    attackSliderAttachment = std::make_unique<SliderAttachment> (processorRef.getState(), "attack", attackSlider);
    decaySliderAttachment = std::make_unique<SliderAttachment> (processorRef.getState(), "decay", decaySlider);
    sustainSliderAttachment = std::make_unique<SliderAttachment> (processorRef.getState(), "sustain", sustainSlider);
    releaseSliderAttachment = std::make_unique<SliderAttachment> (processorRef.getState(), "release", releaseSlider);

    oscSelectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processorRef.getState(), "osc", oscSelector);

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
    gainSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    //gainSlider.setPopupDisplayEnabled (true, false, this);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, true, 100, 50);
    addAndMakeVisible (gainSlider);

    gainLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    gainLabel.attachToComponent (&gainSlider, false);
    //gainLabel.setJustificationType (juce::Justification::bottom);
    addAndMakeVisible (gainLabel);

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

    //auto area = getLocalBounds();
    //g.setColour (juce::Colours::white);
    //g.setFont (16.0f);
    //auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    //auto area = getLocalBounds();
    //area.removeFromBottom (50);

    const int width = getWidth();
    const int height = getHeight();

    inspectButton.setBounds (width - 100, 80, 80, 40);

    // MidiKeyboardComponent
    const int keyboardHeight = 100;
    midiKeyboard.setBounds (0, height - keyboardHeight, width, keyboardHeight);

    // Undo and redo buttons
    const int undoButtonSize = 30;
    undoButton.setBounds (width - (undoButtonSize * 2 + 30), 30, undoButtonSize, undoButtonSize);
    redoButton.setBounds (width - (undoButtonSize + 20), 30, undoButtonSize, undoButtonSize);

    gainSlider.setBounds (40, 50, 40, height / 4);
    
}