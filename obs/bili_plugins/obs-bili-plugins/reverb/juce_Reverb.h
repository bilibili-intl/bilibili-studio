/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/
#ifndef JUCE_REVERB_H_INCLUDED
#define JUCE_REVERB_H_INCLUDED
#include <Windows.h>
#include "juce_HeapBlock.h"

//==============================================================================
/**
    Performs a simple reverb effect on a stream of audio data.

    This is a simple stereo reverb, based on the technique and tunings used in FreeVerb.
    Use setSampleRate() to prepare it, and then call processStereo() or processMono() to
    apply the reverb to your audio data.

    @see ReverbAudioSource
*/
enum
{
	ROOMSIZE,
	DAMPING,
	WETLEVEL,
	DRYLEVEL,
	WIDTH,
	FREEZEMODE,
};

class Reverb
{
public:
    //==============================================================================
    Reverb()
    {
        setParameters (Parameters());
        setSampleRate (44100.0);
    }

    //==============================================================================
    /** Holds the parameters being used by a Reverb object. */
    struct Parameters
    {
		Parameters() throw()
			: roomSize   (0.7f),
			damping    (0.6f),
			wetLevel   (0.6f),
			dryLevel   (0.8f),
			width      (1.0f),
			freezeMode (0)
		{}

        float roomSize;     /**< Room size, 0 to 1.0, where 1.0 is big, 0 is small. */
        float damping;      /**< Damping, 0 to 1.0, where 0 is not damped, 1.0 is fully damped. */
        float wetLevel;     /**< Wet level, 0 to 1.0 */
        float dryLevel;     /**< Dry level, 0 to 1.0 */
        float width;        /**< Reverb width, 0 to 1.0, where 1.0 is very wide. */
        float freezeMode;   /**< Freeze mode - values < 0.5 are "normal" mode, values > 0.5
                                 put the reverb into a continuous feedback loop. */
    };

    //==============================================================================
    /** Returns the reverb's current parameters. */
    const Parameters& getParameters() const throw()    { return parameters; }

    /** Applies a new set of parameters to the reverb.
        Note that this doesn't attempt to lock the reverb, so if you call this in parallel with
        the process method, you may get artifacts.
    */
    void setParameters (const Parameters& newParams)
    {
        const float wetScaleFactor = 3.0f;
        const float dryScaleFactor = 2.0f;

        const float wet = newParams.wetLevel * wetScaleFactor;
        dryGain.setValue (newParams.dryLevel * dryScaleFactor);
        wetGain1.setValue (0.5f * wet * (1.0f + newParams.width));
        wetGain2.setValue (0.5f * wet * (1.0f - newParams.width));

        gain = isFrozen (newParams.freezeMode) ? 0.0f : 0.015f;
        parameters = newParams;
        updateDamping();
    }

    //==============================================================================
    /** Sets the sample rate that will be used for the reverb.
        You must call this before the process methods, in order to tell it the correct sample rate.
    */
    void setSampleRate (const double sampleRate)
    {
        assert (sampleRate > 0);

        static const short combTunings[] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 }; // (at 44100Hz)
        static const short allPassTunings[] = { 556, 441, 341, 225 };
        const int stereoSpread = 23;
        const int intSampleRate = (int) sampleRate;

        for (int i = 0; i < numCombs; ++i)
        {
            comb[0][i].setSize ((intSampleRate * combTunings[i]) / 44100);
            comb[1][i].setSize ((intSampleRate * (combTunings[i] + stereoSpread)) / 44100);
        }

        for (int i = 0; i < numAllPasses; ++i)
        {
            allPass[0][i].setSize ((intSampleRate * allPassTunings[i]) / 44100);
            allPass[1][i].setSize ((intSampleRate * (allPassTunings[i] + stereoSpread)) / 44100);
        }

        const double smoothTime = 0.01;
        damping .reset (sampleRate, smoothTime);
        feedback.reset (sampleRate, smoothTime);
        dryGain .reset (sampleRate, smoothTime);
        wetGain1.reset (sampleRate, smoothTime);
        wetGain2.reset (sampleRate, smoothTime);
    }

    /** Clears the reverb's buffers. */
    void reset()
    {
        for (int j = 0; j < numChannels; ++j)
        {
            for (int i = 0; i < numCombs; ++i)
                comb[j][i].clear();

            for (int i = 0; i < numAllPasses; ++i)
                allPass[j][i].clear();
        }
    }

    //==============================================================================
    /** Applies the reverb to two stereo channels of audio data. */
    void processStereo (float* const left, float* const right, const int numSamples) throw()
    {
        assert (left != nullptr && right != nullptr);

        for (int i = 0; i < numSamples; ++i)
        {
            const float input = (left[i] + right[i]) * gain;
            float outL = 0, outR = 0;

            const float damp    = damping.getNextValue();
            const float feedbck = feedback.getNextValue();

            for (int j = 0; j < numCombs; ++j)  // accumulate the comb filters in parallel
            {
                outL += comb[0][j].process (input, damp, feedbck);
                outR += comb[1][j].process (input, damp, feedbck);
            }

            for (int j = 0; j < numAllPasses; ++j)  // run the allpass filters in series
            {
                outL = allPass[0][j].process (outL);
                outR = allPass[1][j].process (outR);
            }

            const float dry  = dryGain.getNextValue();
            const float wet1 = wetGain1.getNextValue();
            const float wet2 = wetGain2.getNextValue();

            left[i]  = outL * wet1 + outR * wet2 + left[i]  * dry;
            right[i] = outR * wet1 + outL * wet2 + right[i] * dry;
        }
    }


	void processStereo (float* const left, float* const right, const int numSamples,BYTE* outLeft,BYTE* outRight) throw()
	{
		assert (left != nullptr && right != nullptr);

		for (int i = 0; i < numSamples; ++i)
		{
			const float input = (left[i] + right[i]) * gain;
			float outL = 0, outR = 0;

			const float damp    = damping.getNextValue();
			const float feedbck = feedback.getNextValue();

			for (int j = 0; j < numCombs; ++j)  // accumulate the comb filters in parallel
			{
				outL += comb[0][j].process (input, damp, feedbck);
				outR += comb[1][j].process (input, damp, feedbck);
			}

			for (int j = 0; j < numAllPasses; ++j)  // run the allpass filters in series
			{
				outL = allPass[0][j].process (outL);
				outR = allPass[1][j].process (outR);
			}

			const float dry  = dryGain.getNextValue();
			const float wet1 = wetGain1.getNextValue();
			const float wet2 = wetGain2.getNextValue();

			outLeft[i] = outL * wet1 + outR * wet2 + left[i]  * dry;
			outRight[i] = outR * wet1 + outL * wet2 + right[i] * dry;
			//left[i]  = outL * wet1 + outR * wet2 + left[i]  * dry;
			//right[i] = outR * wet1 + outL * wet2 + right[i] * dry;
		}
	}
    /** Applies the reverb to a single mono channel of audio data. */
    void processMono (float* const samples, const int numSamples) throw()
    {
        assert (samples != nullptr);
        for (int i = 0; i < numSamples; ++i)
        {
            const float input = samples[i] * gain;
            float output = 0;

            const float damp    = damping.getNextValue();
            const float feedbck = feedback.getNextValue();

            for (int j = 0; j < numCombs; ++j)  // accumulate the comb filters in parallel
                output += comb[0][j].process (input, damp, feedbck);

            for (int j = 0; j < numAllPasses; ++j)  // run the allpass filters in series
                output = allPass[0][j].process (output);

            const float dry  = dryGain.getNextValue();
            const float wet1 = wetGain1.getNextValue();

            samples[i] = output * wet1 + samples[i] * dry;
        }
    }

private:
    //==============================================================================
    static bool isFrozen (const float freezeMode) throw()  { return freezeMode >= 0.5f; }

    void updateDamping() throw()
    {
        const float roomScaleFactor = 0.28f;
        const float roomOffset = 0.7f;
        const float dampScaleFactor = 0.4f;

        if (isFrozen (parameters.freezeMode))
            setDamping (0.0f, 1.0f);
        else
            setDamping (parameters.damping * dampScaleFactor,
                        parameters.roomSize * roomScaleFactor + roomOffset);
    }

    void setDamping (const float dampingToUse, const float roomSizeToUse) throw()
    {
        damping.setValue (dampingToUse);
        feedback.setValue (roomSizeToUse);
    }

    //==============================================================================
    class CombFilter
    {
    public:
        CombFilter() throw()   : bufferSize (0), bufferIndex (0), last (0)  {}

        void setSize (const int size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc ((size_t) size);
                bufferSize = size;
            }

            clear();
        }

        void clear() throw()
        {
            last = 0;
            buffer.clear ((size_t) bufferSize);
        }

        float process (const float input, const float damp, const float feedbackLevel) throw()
        {
            const float output = buffer[bufferIndex];
            last = (output * (1.0f - damp)) + (last * damp);
            JUCE_UNDENORMALISE (last);

            float temp = input + (last * feedbackLevel);
            JUCE_UNDENORMALISE (temp);
            buffer[bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return output;
        }

    private:
        HeapBlock<float> buffer;
        int bufferSize, bufferIndex;
        float last;

       // JUCE_DECLARE_NON_COPYABLE (CombFilter)
    };

    //==============================================================================
    class AllPassFilter
    {
    public:
        AllPassFilter() throw()  : bufferSize (0), bufferIndex (0) {}

        void setSize (const int size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc ((size_t) size);
                bufferSize = size;
            }

            clear();
        }

        void clear() throw()
        {
            buffer.clear ((size_t) bufferSize);
        }

        float process (const float input) throw()
        {
            const float bufferedValue = buffer [bufferIndex];
            float temp = input + (bufferedValue * 0.5f);
            JUCE_UNDENORMALISE (temp);
            buffer [bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return bufferedValue - input;
        }

    private:
        HeapBlock<float> buffer;
        int bufferSize, bufferIndex;

        //JUCE_DECLARE_NON_COPYABLE (AllPassFilter)
    };

    //==============================================================================
    class LinearSmoothedValue
    {
    public:
        LinearSmoothedValue() throw()
            : currentValue (0), target (0), step (0), countdown (0), stepsToTarget (0)
        {
        }

        void reset (double sampleRate, double fadeLengthSeconds) throw()
        {
            assert (sampleRate > 0 && fadeLengthSeconds >= 0);
            stepsToTarget = (int) std::floor (fadeLengthSeconds * sampleRate);
            currentValue = target;
            countdown = 0;
        }

        void setValue (float newValue) throw()
        {
            if (target != newValue)
            {
                target = newValue;
                countdown = stepsToTarget;

                if (countdown <= 0)
                    currentValue = target;
                else
                    step = (target - currentValue) / (float) countdown;
            }
        }

        float getNextValue() throw()
        {
            if (countdown <= 0)
                return target;

            --countdown;
            currentValue += step;
            return currentValue;
        }

    private:
        float currentValue, target, step;
        int countdown, stepsToTarget;

        //JUCE_DECLARE_NON_COPYABLE (LinearSmoothedValue)
    };

    //==============================================================================
    enum { numCombs = 8, numAllPasses = 4, numChannels = 2 };

    Parameters parameters;
    float gain;

    CombFilter comb [numChannels][numCombs];
    AllPassFilter allPass [numChannels][numAllPasses];

    LinearSmoothedValue damping, feedback, dryGain, wetGain1, wetGain2;
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reverb)
};


#endif   // JUCE_REVERB_H_INCLUDED
