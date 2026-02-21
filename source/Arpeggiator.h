#pragma once

class Arpeggiator
{
public:
    void prepareToPlay (double sampleRate,
        std::atomic<float>* noteDurPtr,
        std::atomic<float>* noteDurSyncPtr,
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
        noteDurSync = noteDurSyncPtr;
        randomize = randomizePtr;
        density = densityPtr;
        width = widthPtr;
        pan = panPtr;
        ascending = ascendingPtr;
        sync = syncPtr;
    };

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi, juce::Optional<juce::AudioPlayHead::PositionInfo>& infoOpt)
    {
        auto bufferSamples = buffer.getNumSamples();

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

        // Note duration in seconds
        float noteDuration = noteDur->load();

        // Note duration in samples
        int noteDurSamples = static_cast<int> (noteDuration * sr);

        // Midi event sample offset
        int offset = juce::jmax (0, juce::jmin ((int) (noteDurSamples - samples), bufferSamples - 1));

        // Condition to check when to send midi event
        bool condition = (samples + bufferSamples) >= noteDurSamples;

        if (sync->load() && infoOpt.hasValue())
        {
            juce::AudioPlayHead::PositionInfo info = *infoOpt;
            auto bpm = info.getBpm();
            auto ppqPosition = info.getPpqPosition();

            if (bpm && ppqPosition)
            {
                int noteDurSyncIndex = static_cast<int> (noteDurSync->load());
                int noteDenominator = std::pow (2, 7 - noteDurSyncIndex);
                float quarterNoteSamples = 60.0f / *bpm * sr ;

                if (info.getIsPlaying())
                {
                    float bufferPpq = bufferSamples / quarterNoteSamples;
                    float ppqFmod = std::fmod (*ppqPosition + bufferPpq, 4.0f / noteDenominator);

                    condition = ppqFmod > 0 && ppqFmod < prevPpqFmod;
                    offset = static_cast<int> ((bufferPpq - ppqFmod) * quarterNoteSamples);

                    prevPpqFmod = ppqFmod;
                }
                else
                {
                    noteDurSamples = quarterNoteSamples * 4 / noteDenominator;

                    offset = juce::jmax (0, juce::jmin ((int) (noteDurSamples - samples), bufferSamples - 1));
                    condition = (samples + bufferSamples) >= noteDurSamples;

                    // Send MIDI note off event if playback stops
                    if (condition && lastNoteValue > 0)
                    {
                        midi.addEvent (juce::MidiMessage::noteOff (1, lastNoteValue), 0);
                        lastNoteValue = -1;
                    }
                }
            }
        }

        if (condition)
        {
            if (lastNoteValue > 0)
            {
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
    std::atomic<float>* noteDurSync;
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
    float prevPpqFmod = 0.0f;

    float sr { 0.0f };
};