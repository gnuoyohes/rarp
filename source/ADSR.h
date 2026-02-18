#pragma once

// Modified version of JUCE's ADSR envelope class

//==============================================================================
/**
A very simple ADSR envelope class.

To use it, call setSampleRate() with the current sample rate and give it some parameters
with setParameters() then call getNextSample() to get the envelope value to be applied
to each audio sample or applyEnvelopeToBuffer() to apply the envelope to a whole buffer.

Do not change the parameters during playback. If you change the parameters before the
release stage has completed then you must call reset() before the next call to
noteOn().

@tags{Audio}
*/
class ADSR
{
public:
    //==============================================================================
    ADSR()
    {
        //recalculateRates();
    }

    //==============================================================================
    
    /**
    Static function that maps domain [0, 1] -> range [0, 1] using an exponential curve
    (used in getNextSample())

    */
    static float curve (float x, float expo, bool ascending)
    {
        if (ascending == true)
            return (std::exp (expo * x) - 1) / (std::exp (expo) - 1);
        else
            return (std::exp (-1 * expo * (x - 1)) - 1) / (std::exp (expo) - 1);
    }

    //==============================================================================
    /**
    Holds the parameters being used by an ADSR object.

    @tags{Audio}
    */
    struct Parameters
    {
        Parameters() = default;

        Parameters (float attackTimeSeconds,
            float decayTimeSeconds,
            float sustainLevel,
            float releaseTimeSeconds,
            float expoVal)
            : attack (attackTimeSeconds),
              decay (decayTimeSeconds),
              sustain (sustainLevel),
              release (releaseTimeSeconds),
              expo (expoVal)

        {
        }

        float attack = 0.1f, decay = 0.1f, sustain = 1.0f, release = 0.1f, expo = 0.1f;
    };

    /** Sets the parameters that will be used by an ADSR object.

    You must have called setSampleRate() with the correct sample rate before
    this otherwise the values may be incorrect!

    @see getParameters
    */
    void setParameters (const Parameters& newParameters)
    {
        // need to call setSampleRate() first!
        jassert (sampleRate > 0.0);

        parameters = newParameters;
        //recalculateRates();
    }

    /** Returns the parameters currently being used by an ADSR object.

    @see setParameters
    */
    const Parameters& getParameters() const noexcept { return parameters; }

    /** Returns true if the envelope is in its attack, decay, sustain or release stage. */
    bool isActive() const noexcept { return state != State::idle; }

    //==============================================================================
    /** Sets the sample rate that will be used for the envelope.

    This must be called before the getNextSample() or setParameters() methods.
    */
    void setSampleRate (double newSampleRate) noexcept
    {
        jassert (newSampleRate > 0.0);
        sampleRate = newSampleRate;
    }

    //==============================================================================
    /** Resets the envelope to an idle state. */
    void reset() noexcept
    {
        envelopeVal = 0.0f;
        numSamples = 0;
        releaseVal = 0.0f;
        state = State::idle;
    }

    /** Starts the attack phase of the envelope. */
    void noteOn() noexcept
    {
        if (parameters.attack > 0.0f)
        {
            numSamples = parameters.attack * sampleRate;
            state = State::attack;
        }
        else if (parameters.decay > 0.0f)
        {
            //envelopeVal = 1.0f;
            numSamples = parameters.decay * sampleRate;
            state = State::decay;
        }
        else
        {
            numSamples = 0;
            //envelopeVal = parameters.sustain;
            state = State::sustain;
        }
    }

    /** Starts the release phase of the envelope. */
    void noteOff() noexcept
    {
        if (state != State::idle)
        {
            if (parameters.release > 0.0f)
            {
                //releaseRate = (float) (envelopeVal / (parameters.release * sampleRate));
                sampleDelta = 0;
                releaseVal = envelopeVal;
                numSamples = parameters.release * sampleRate;
                state = State::release;
            }
            else
            {
                reset();
            }
        }
    }

    //==============================================================================
    /** Returns the next sample value for an ADSR object.

    @see applyEnvelopeToBuffer
    */
    float getNextSample() noexcept
    {
        switch (state)
        {
            case State::idle:
            {
                return 0.0f;
            }

            case State::attack:
            {
                //envelopeVal += attackRate;
                envelopeVal = curve (sampleDelta / numSamples, parameters.expo, true);

                if (sampleDelta > numSamples)
                {
                    //envelopeVal = 1.0f;
                    goToNextState();
                }

                break;
            }

            case State::decay:
            {
                //envelopeVal -= decayRate;
                envelopeVal = curve (sampleDelta / numSamples, parameters.expo, false) * (1.0 - parameters.sustain) + parameters.sustain;

                if (sampleDelta > numSamples)
                {
                    //envelopeVal = parameters.sustain;
                    goToNextState();
                }

                break;
            }

            case State::sustain:
            {
                envelopeVal = parameters.sustain;
                break;
            }

            case State::release:
            {
                //envelopeVal -= releaseRate;
                envelopeVal = curve (sampleDelta / numSamples, parameters.expo, false) * releaseVal;

                if (sampleDelta > numSamples)
                {
                    goToNextState();
                }
                break;
            }
        }

        ++sampleDelta;

        return envelopeVal;
    }

    /** This method will conveniently apply the next numSamples number of envelope values
    to an AudioBuffer.

    @see getNextSample
    */
    void applyEnvelopeToBuffer (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        jassert (startSample + numSamples <= buffer.getNumSamples());

        if (state == State::idle)
        {
            buffer.clear (startSample, numSamples);
            return;
        }

        if (state == State::sustain)
        {
            buffer.applyGain (startSample, numSamples, parameters.sustain);
            return;
        }

        auto numChannels = buffer.getNumChannels();

        while (--numSamples >= 0)
        {
            auto env = getNextSample();

            for (int i = 0; i < numChannels; ++i)
                buffer.getWritePointer (i)[startSample] *= env;

            ++startSample;
        }
    }

    void initialize (std::array<std::atomic<float>*, 5> adsrPtrs)
    {
        atomicParams = adsrPtrs;
    }

    void updateADSR()
    {
        Parameters params;

        params.attack = atomicParams[0]->load();
        params.decay = atomicParams[1]->load();
        params.sustain = atomicParams[2]->load();
        params.release = atomicParams[3]->load();
        params.expo = atomicParams[4]->load();

        setParameters (params);
    }

private:
    //==============================================================================
    //void recalculateRates() noexcept
    //{
    //    auto getRate = [] (float distance, float timeInSeconds, double sr) {
    //        return timeInSeconds > 0.0f ? (float) (distance / (timeInSeconds * sr)) : -1.0f;
    //    };

    //    attackRate = getRate (1.0f, parameters.attack, sampleRate);
    //    decayRate = getRate (1.0f - parameters.sustain, parameters.decay, sampleRate);
    //    releaseRate = getRate (parameters.sustain, parameters.release, sampleRate);

    //    if ((state == State::attack && attackRate <= 0.0f)
    //        || (state == State::decay && (decayRate <= 0.0f || envelopeVal <= parameters.sustain))
    //        || (state == State::release && releaseRate <= 0.0f))
    //    {
    //        goToNextState();
    //    }
    //}

    void goToNextState() noexcept
    {
        sampleDelta = 0;

        if (state == State::attack)
        {
            numSamples = parameters.decay * sampleRate;
            state = (parameters.decay > 0.0f ? State::decay : State::sustain);
            return;
        }

        if (state == State::decay)
        {
            numSamples = 0;
            state = State::sustain;
            return;
        }

        if (state == State::release)
            reset();
    }

    //==============================================================================
    enum class State { idle,
        attack,
        decay,
        sustain,
        release };

    State state = State::idle;
    Parameters parameters;
    std::array<std::atomic<float>*, 5> atomicParams;

    double sampleRate = 44100.0;
    float envelopeVal = 0.0f;

    // Variable to store envelope value at time of release
    float releaseVal = 0.0f;

    // Variables to keep track of which sample we are at in a state
    int sampleDelta = 0;
    double numSamples = 0;

    //float attackRate = 0.0f, decayRate = 0.0f, releaseRate = 0.0f;
};