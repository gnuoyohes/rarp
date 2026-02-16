#include "SynthVoice.h"

SynthVoice::SynthVoice (std::atomic<float>* gainPtr)
    : gainParam(gainPtr)
{
}

//==============================================================================

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    auto freq = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);

    // Force = true, no pitch glide
    sinOsc.setFrequency (freq, true);

    adsr.noteOn();
}
void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    adsr.noteOff();

    if (!allowTailOff || !adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{
    //DBG (newPitchWheelValue);
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    return;
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    voiceBuffer.setSize (outputBuffer.getNumChannels(), numSamples, false, false, true);
    voiceBuffer.clear();

    juce::dsp::AudioBlock<float> audioBlock { voiceBuffer };
    //auto subBlock = audioBlock.getSubBlock ((size_t) startSample, (size_t) numSamples);
    //juce::dsp::ProcessContextReplacing<float> context { subBlock };
    juce::dsp::ProcessContextReplacing<float> context { audioBlock };

    // Apply sine oscillator
    sinOsc.process (context);

    // Get gain value from PluginProcessor's atomic float and apply
    gain.setGainLinear (gainParam->load());
    gain.process (context);

    // Apply ADSR Envelope
    adsr.applyEnvelopeToBuffer(voiceBuffer, 0, voiceBuffer.getNumSamples());

    for (int i = 0; i < outputBuffer.getNumChannels(); ++i)
        outputBuffer.addFrom (i, startSample, voiceBuffer, i, 0, numSamples);

    if (!adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    adsr.setSampleRate(sampleRate);
    adsrParams.attack = 0.1f;
    adsrParams.decay = 0.4f;
    adsrParams.sustain = 0.3f;
    adsrParams.release = 0.3f;
    adsr.setParameters (adsrParams);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = numOutputChannels;

    sinOsc.prepare (spec);
    gain.prepare (spec);
    gain.setRampDurationSeconds (0.01);
}