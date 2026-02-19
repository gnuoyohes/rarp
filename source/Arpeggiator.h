#pragma once

class Arpeggiator
{
public:
    void prepareToPlay (double sampleRate,
        std::atomic<float>* noteDurPtr,
        std::atomic<float>* randomizePtr,
        std::atomic<float>* densityPtr,
        std::atomic<float>* widthPtr,
        std::atomic<float>* panPtr,
        std::atomic<float>* ascendingPtr,
        std::atomic<float>* syncPtr)
    {
        notes.clear();
        sr = static_cast<float> (sampleRate);

        noteDur = noteDurPtr;
        randomize = randomizePtr;
        density = densityPtr;
        width = widthPtr;
        pan = panPtr;
        ascending = ascendingPtr;
        sync = syncPtr;
    };

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
    {
        auto bufferSamples = buffer.getNumSamples();

        // Note duration in seconds
        float noteDuration = noteDur->load();

        // Note duration in samples
        int noteDurSamples = static_cast<int> (noteDuration * sr);

        // Process midi
        for (const auto metadata : midi)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                notes.add (msg.getNoteNumber());
            else if (msg.isNoteOff())
                notes.removeValue (msg.getNoteNumber());
        }
        midi.clear();

        if ((samples + bufferSamples) >= noteDurSamples)
        {
            auto offset = juce::jmax (0, juce::jmin ((int) (noteDuration - samples), bufferSamples - 1));
            
            if (lastNoteValue > 0)
            {
                auto offset = juce::jmax (0, juce::jmin ((int) (noteDuration / 2 - samples), bufferSamples - 1));
                midi.addEvent (juce::MidiMessage::noteOff (1, lastNoteValue), offset);
                lastNoteValue = -1;
            }

            if (notes.size() > 0 && random.nextFloat() < density->load())
            {
                // Set pan of current note, processed in PluginProcessor's processBlock()
                float widthVal = width->load();
                pan->store(random.nextFloat() * widthVal * 2 - widthVal);

                // Select and play note from notes
                if (ascending->load())
                    currentNote = (currentNote + 1) % notes.size();
                else
                {
                    currentNote -= 1;
                    if (currentNote < 0)
                        currentNote = notes.size() - 1;
                }

                if (random.nextFloat() < randomize->load())
                {
                    int randomNoteIndex = random.nextInt (notes.size());
                    lastNoteValue = notes[randomNoteIndex];
                }
                else
                {
                    lastNoteValue = notes[currentNote];
                }

                midi.addEvent (juce::MidiMessage::noteOn (1, lastNoteValue, (juce::uint8) 127), offset);
            }
        }
        samples = (samples + bufferSamples) % noteDurSamples;
    };

private:
    std::atomic<float>* noteDur;
    std::atomic<float>* randomize;
    std::atomic<float>* density;
    std::atomic<float>* width;
    std::atomic<float>* pan;
    std::atomic<float>* ascending;
    std::atomic<float>* sync;

    juce::Random random;

    juce::SortedSet<int> notes;
    int samples = 0;
    int currentNote = 0; // index of currently playing note in notes
    int lastNoteValue = -1; // midi value of last played note
    float sr { 0.0f };
};