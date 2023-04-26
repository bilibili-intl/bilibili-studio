#include "reverb.h"
#include <iostream>
#include <assert.h>
#include "juce_Reverb.h"

extern "C" 
{
    Reverb* bili_reverb_create()
    {
        return new Reverb();
    }

    void bili_reverb_destroy(Reverb* instance)
    {
        delete instance;
    }

    void bili_reverb_set_sample_rate(Reverb* instance, double val)
    {
        instance->setSampleRate(val);
    }

    void bili_reverb_reset(Reverb* instance)
    {
        instance->reset();
    }

    void bili_reverb_process(Reverb* instance, float *left, float *right, int numSample)
    {
        if (!right)
            instance->processMono(left, numSample);
        else
            instance->processStereo(left, right, numSample);
    }

    void bili_reverb_set_parameters(Reverb* instance, struct ReverbParameters *val)
    {
        Reverb::Parameters param;
        param.roomSize = val->roomSize;
        param.damping = val->damping;
        param.dryLevel = val->dryLevel;
        param.freezeMode = val->freezeMode;
        param.wetLevel = val->wetLevel;
        param.width = val->width;
        instance->setParameters(param);
    }
};
