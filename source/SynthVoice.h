#pragma once

#include "SynthSound.h"
#include <juce_dsp/juce_dsp.h>

#include "ADSR.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice (std::atomic<float>* gainPtr, std::array<std::atomic<float>*, 5> adsrPtrs, std::atomic<float>* oscPtr);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock, int numOutputChannels);

private:

    // Per-voice audio buffer to be processed by this voice before being added to outputBuffer
    // in renderNextBlock to prevent popping artifacts while supporting polyphony
    juce::AudioBuffer<float> voiceBuffer;

    ADSR adsr;

    // DSP modules
    std::array<juce::dsp::Oscillator<float>, 4> oscillators;

    juce::dsp::Gain<float> gain;

    // Atomic param ptrs passed from PluginProcessor
    std::atomic<float>* gainAtomic;
    std::atomic<float>* oscAtomic;
};