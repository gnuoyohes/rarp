#include "SynthVoice.h"

SynthVoice::SynthVoice (std::atomic<float>* gainPtr, std::array<std::atomic<float>*, 5> adsrPtrs, std::atomic<float>* oscPtr)
    : gainAtomic (gainPtr),
      oscAtomic (oscPtr)
{
    oscillators = {
        // sine
        juce::dsp::Oscillator<float> ([] (float x) { return std::sin (x); }),
        // triangle
        juce::dsp::Oscillator<float> ([] (float x) { return 2 / juce::MathConstants<float>::pi * std::asin (std::sin (x)); }),
        // sawtooth
        juce::dsp::Oscillator<float> ([] (float x) { return x / juce::MathConstants<float>::pi; }),
        // square
        juce::dsp::Oscillator<float> ([] (float x) { return x < 0.0f ? -1.0f : 1.0f; })
    };
    adsr.initialize (adsrPtrs);
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
    for (int i = 0; i < oscillators.size(); ++i)
    {
        oscillators[i].setFrequency (freq, true);
    }

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
    juce::dsp::ProcessContextReplacing<float> context { audioBlock };

    // This method does not work with polyphony
    //auto subBlock = audioBlock.getSubBlock ((size_t) startSample, (size_t) numSamples);
    //juce::dsp::ProcessContextReplacing<float> context { subBlock };

    // Process oscillator
    int oscIndex = static_cast<int> (oscAtomic->load());
    oscillators[oscIndex].process (context);
    

    // Get gain value from PluginProcessor's atomic float and apply
    gain.setGainLinear (gainAtomic->load());
    gain.process (context);

    // Apply ADSR Envelope
    adsr.updateADSR();
    adsr.applyEnvelopeToBuffer(voiceBuffer, 0, voiceBuffer.getNumSamples());

    // Add from per-voice voiceBuffer to outputBuffer from startSample
    for (int i = 0; i < outputBuffer.getNumChannels(); ++i)
        outputBuffer.addFrom (i, startSample, voiceBuffer, i, 0, numSamples);

    if (!adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    adsr.setSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = numOutputChannels;

    for (int i = 0; i < oscillators.size(); ++i)
    {
        oscillators[i].prepare (spec);
    }

    gain.prepare (spec);
    gain.setRampDurationSeconds (0.01);
}