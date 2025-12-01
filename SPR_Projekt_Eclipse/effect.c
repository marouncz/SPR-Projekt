/*
 * effect.c
 *
 *  Created on: 27. 11. 2025
 *      Author: Student
 */


#include "effect.h"
#include <std.h>

#define FLOAT2FIXED(x)  ((x) >= 0 ? ((Int16)((x)*32768+0.5)) : ((Int16)((x)*32768-0.5)))

#define BUFFER_SIZE (1<<14) //16384 samples
#define SAMPLE_FREQ (48000)

Int32 delay( Int16 sample, Uint16 delayMs)
{

}

//delay value is in ms to not use floats
Int16 echoProcessing( Int16 inputSample, Uint16 delayMs, float alpha_coeff, float beta_coeff)
{

    static Int32 buffer[BUFFER_SIZE] = {0};
    static Uint32 writePtr = 0;

    //Int32 alpha = FLOAT2FIXED(alpha_coeff);
    //Int32 beta  = FLOAT2FIXED(beta_coeff);

    Int32 alpha = FLOAT2FIXED(0.6);
    Int32 beta  = FLOAT2FIXED(0.6);

    // Convert input to Q15 stored in upper 16 bits of 32-bit word
    Int32 x = ((Int32)inputSample) << 16;

    // Compute delay in samples
    Uint32 delaySamples = SAMPLE_FREQ * delayMs / 1000;


    if (delaySamples >= BUFFER_SIZE)
        delaySamples = BUFFER_SIZE - 1;

    // Compute read pointer (circular)
    Uint32 readPtr = writePtr + BUFFER_SIZE - delaySamples;
    if (readPtr >= BUFFER_SIZE)
        readPtr -= BUFFER_SIZE;

    Int32 d_delay = buffer[readPtr];



    // s = alpha * delayed sample
    Int32 s = _smpylh(alpha, d_delay);

    // y = x + s
    Int32 y = _sadd(x, s);

    // d = x + beta * s
    Int32 bs = _smpylh(beta, s);
    Int32 d = _sadd(x, bs);

    // Store d in circular buffer
    buffer[writePtr] = d;

    // Increment write pointer
    writePtr++;
    if (writePtr >= BUFFER_SIZE)
        writePtr = 0;

    // Convert back to Int16 for output
    Int16 outSample = (Int16)(y >> 16);  // high 16 bits

    return outSample;
}
