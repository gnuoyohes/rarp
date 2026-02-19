#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "Arpeggiator.h"

#if (MSVC)
#include "ipps.h"
#endif


class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() { return state; }
    juce::UndoManager& getUndoManager() { return undoManager; }
    juce::MidiKeyboardState& getMidiKeyboardState() { return keyboardState; }

    juce::AudioVisualiserComponent waveform { 2 };

private:
    static juce::String msValueToTextFunction (float value, int maximumStringLength)
    {
        if (value < 1)
            return juce::String (value * 1000.0f) + " ms";
        else
            return juce::String (value) + " s";
    };

    static float msTextToValueFunction (const juce::String& text)
    {
        return text.getFloatValue() / 1000.0f;
    };

    juce::AudioProcessorValueTreeState state;
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    juce::Synthesiser synth;

    Arpeggiator arp;
    std::atomic<float> pan { 0.0f }; // from -1.0 (left) to 1.0 (right)
    float prevLeftGain { 0 };
    float prevRightGain { 0 };

    juce::MidiKeyboardState keyboardState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
