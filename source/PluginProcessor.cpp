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
    // Initialize atomic param ptrs
    std::atomic<float>* gainAtomic = state.getRawParameterValue ("gain");
    std::array<std::atomic<float>*, 5> adsrAtomic = {
        state.getRawParameterValue ("attack"),
        state.getRawParameterValue ("decay"),
        state.getRawParameterValue ("sustain"),
        state.getRawParameterValue ("release"),
        state.getRawParameterValue ("expo")
    };
    std::atomic<float>* oscAtomic = state.getRawParameterValue ("osc");
    
    synth.addSound (new SynthSound());

    const int numSynthVoices = 8;

    for (int i = 0; i < numSynthVoices; ++i)
        synth.addVoice (new SynthVoice (gainAtomic, adsrAtomic, oscAtomic));

    //waveform.setBufferSize(64);
    waveform.setSamplesPerBlock(128);
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

    // Prepare arpeggiator
    arp.prepareToPlay (sampleRate,
        state.getRawParameterValue ("noteDur"),
        state.getRawParameterValue ("randomize"),
        state.getRawParameterValue ("density"),
        state.getRawParameterValue ("width"),
        &pan,
        state.getRawParameterValue ("ascending"),
        state.getRawParameterValue ("sync")
    );

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

    int numSamples = buffer.getNumSamples();

    // Process MIDI messages
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // Process arpeggiator
    arp.processBlock (buffer, midiMessages);

    // Process synth block
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    // Pan output
    float panVal = pan.load();

    if (panVal != 0.0f)
    {
        //auto* leftChannel = buffer.getWritePointer (0);
        //auto* rightChannel = buffer.getWritePointer (1);
        float leftGain = std::cos (0.5 * (pan + 1.0) * 0.5 * juce::MathConstants<float>::pi);
        float rightGain = std::sin (0.5 * (pan + 1.0) * 0.5 * juce::MathConstants<float>::pi);

        buffer.applyGainRamp (0, 0, numSamples, prevLeftGain, leftGain);
        buffer.applyGainRamp (1, 0, numSamples, prevRightGain, rightGain);

        prevLeftGain = leftGain;
        prevRightGain = rightGain;
    }

    waveform.pushBuffer (buffer);
    
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
        juce::StringArray {"Sine", "Triangle", "Saw", "Square"},
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
        juce::NormalisableRange<float>(0.001f, 1.0f, 0.001, 0.5),
        0.005f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        &msValueToTextFunction,
        &msTextToValueFunction
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "decay" },
        "Decay",
        juce::NormalisableRange<float> (0.001f, 1.0f, 0.001, 0.5),
        0.005f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        &msValueToTextFunction,
        &msTextToValueFunction
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "sustain" },
        "Sustain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01),
        1.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "release" },
        "Release",
        juce::NormalisableRange<float> (0.001f, 1.0f, 0.001, 0.5),
        0.005f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        &msValueToTextFunction,
        &msTextToValueFunction
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "expo" },
        "Expo",
        0.1f,
        10.0f,
        0.1f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "noteDur" },
        "Note Duration",
        juce::NormalisableRange<float> (0.001f, 3.0f, 0.001, 0.3),
        0.1f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        &msValueToTextFunction,
        &msTextToValueFunction
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "randomize" },
        "Randomize",
        0.0f,
        1.0f,
        0.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "density" },
        "Density",
        0.0f,
        1.0f,
        1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "width" },
        "Width",
        0.0f,
        1.0f,
        0.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "ascending" },
        "Ascending",
        true
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "sync" },
        "Sync with BPM",
        false
    ));

    return { params.begin(), params.end() };
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
