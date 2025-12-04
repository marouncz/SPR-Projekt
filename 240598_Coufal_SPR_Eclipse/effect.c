/*
 * effect_stereo.c
 *
 *  Created on: 27. 11. 2025
 *      Author: Student (stereo version)
 */

#include "effect.h"
#include <std.h>

#define FLOAT2FIXED(x)  ((x) >= 0 ? ((Int16)((x)*32768+0.5)) : ((Int16)((x)*32768-0.5)))

#define BUFFER_SIZE     (1<<14)        // 16384 samples
#define SAMPLE_FREQ     (48000)
#define NUM_CHANNELS    2              // stereo

// initial values for delay effect
static Int32 alpha_fx = FLOAT2FIXED(0.6f);
static Int32 beta_fx  = FLOAT2FIXED(0.6f);
static Uint32 delaySamples = SAMPLE_FREQ * 200 / 1000;
static Int32 normalizationCoeff_fx = FLOAT2FIXED(0.5f);

// stereo delay buffer (2 channels)
static Int32 buffer[NUM_CHANNELS][BUFFER_SIZE] = {{0}};
static Uint32 writePtr[NUM_CHANNELS] = {0, 0};


//delay value is in ms to not use floats
//use this function to change the effect parameters during runtime
EchoStatus echoSetParams(Uint16 delayMs, float alpha_coeff, float beta_coeff)
{
    // Compute delay in samples
    delaySamples = SAMPLE_FREQ * delayMs / 1000;

    if (delaySamples >= BUFFER_SIZE)
    {
        return ECHO_DELAY_OVERFLOW;
    }

    if (alpha_coeff < 0 || beta_coeff < 0)
    {
        return ECHO_NEGATIVE_COEFF;
    }

    if (alpha_coeff * beta_coeff > 1.0f)
    {
        return ECHO_UNSTABLE_COEFF;
    }

    float normalizationCoeff = 1.0f / (1.0f + alpha_coeff/(1.0f + alpha_coeff*beta_coeff) );

    normalizationCoeff_fx = FLOAT2FIXED(normalizationCoeff);
    alpha_fx = FLOAT2FIXED(alpha_coeff);
    beta_fx  = FLOAT2FIXED(beta_coeff);


    return ECHO_OK;
}


// effect processing function - sample by sample
Int16 echoProcessing(Int16 inputSample, Uint16 channel)
{
    // circular read pointer
    Uint32 wp = writePtr[channel];
    Uint32 rp = wp + BUFFER_SIZE - delaySamples;
    if (rp >= BUFFER_SIZE)
        rp -= BUFFER_SIZE;

    Int32 d_delay = buffer[channel][rp];

    // normalize: x = input * normalization
    Int32 x = _smpy(inputSample, normalizationCoeff_fx);

    // s = alpha * delayed sample (high part -> Q15 result)
    Int32 s = _smpylh(alpha_fx, d_delay);

    // y = x + s
    Int32 y = _sadd(x, s);

    // d = x + beta * s (feedback into delay buffer)
    Int32 bs = _smpylh(beta_fx, s);
    Int32 d  = _sadd(x, bs);

    // write back into circular buffer
    buffer[channel][wp] = d;

    wp++;
    if (wp >= BUFFER_SIZE)
        wp = 0;

    writePtr[channel] = wp;

    // output sample (Q15)
    return (Int16)(y >> 16);
}
