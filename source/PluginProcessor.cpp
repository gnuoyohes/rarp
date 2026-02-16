#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
              ),
      state (*this, &undoManager, "parameters", createParameters())
{
    gainParam = state.getRawParameterValue ("gain");

    
    synth.addSound (new SynthSound());

    const int numSynthVoices = 8;

    for (int i = 0; i < numSynthVoices; ++i)
        synth.addVoice (new SynthVoice(gainParam));
}

PluginProcessor::~PluginProcessor()
{ 
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare synth
    synth.setCurrentPlaybackSampleRate (sampleRate);

    // Prepare synth voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        auto voice = dynamic_cast<SynthVoice*> (synth.getVoice (i));
        if (voice)
        {
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //    buffer.clear (i, 0, buffer.getNumSamples());

    buffer.clear();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
    //    auto* channelData = buffer.getWritePointer (channel);
    //    juce::ignoreUnused (channelData);
    //    // ..do something to the data...
    //}

    //for (const auto metadata : midiMessages)
    //{
    //    auto message = metadata.getMessage();
    //    if (message.isNoteOn())
    //    {
    //        int noteNumber = message.getNoteNumber();
    //        freq = juce::MidiMessage::getMidiNoteInHertz (noteNumber);
    //    }
    //    else if (message.isNoteOff())
    //    {
    //        freq = 0.0f;
    //    }
    //}

    // Process MIDI messages
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Process synth block
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
    
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto stateCopy = state.copyState();
    std::unique_ptr<juce::XmlElement> xml (stateCopy.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (state.state.getType()))
            state.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Oscillator select param
    params.push_back(std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "osc" },
        "Oscillator Type",
        juce::StringArray {"Sine", "Saw", "Square"},
        0
    ));

    // Gain param
    params.push_back(std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gain" },
        "Gain",
        0.0f,
        1.0f,
        0.5f
    ));

    // ADSR params
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "attack" },
        "Attack",
        0.0f,
        1.0f,
        0.1f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "decay" },
        "Decay",
        0.0f,
         1.0f,
        0.1f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "sustain" },
        "Sustain",
        0.0f,
        1.0f,
        1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "release" },
        "Release",
        0.1f,
        3.0f,
        0.1f));

    return { params.begin(), params.end() };
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
