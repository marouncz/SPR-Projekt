/*
 * effect.h
 *
 *  Created on: 27. 11. 2025
 *      Author: Student
 */

#ifndef EFFECT_H_
#define EFFECT_H_


#include <std.h>


typedef enum {
    ECHO_OK = 0,              // Success
    ECHO_DELAY_OVERFLOW,      // Delay exceeds buffer size
    ECHO_NEGATIVE_COEFF,      // Alpha or Beta < 0
    ECHO_UNSTABLE_COEFF       // Poles outside unitary circle
} EchoStatus;


EchoStatus echoSetParams(Uint16 delayMs, float alpha_coeff, float beta_coeff);

Int16 echoProcessing(Int16 inputSample, Uint16 channel);


#endif /* EFFECT_H_ */
