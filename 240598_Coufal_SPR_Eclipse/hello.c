/*
 *  Copyright 2010 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

#include <std.h>
#include <log.h>
#include <sts.h>
#include <clk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <dsk6416.h>
#include <dsk6416_led.h>
#include <dsk6416_dip.h>
#include "hellocfg.h"
#include "aic23.h"

#include "effect.h"

static AIC23_ConfigTab CodecCfg = AIC23_DEFAULTCONFIG;

void tskProcess( void)
{
	static Int32 sample;
	Int32 left, right;

	while( 1)
	{
		AIC23_Read( &sample);

		left = _ext( sample, 0, 16);
		right = _ext( sample, 16, 16);

        STS_set(&STS_left, CLK_gethtime());
        left = echoProcessing( left, 0);
        STS_delta( &STS_left, CLK_gethtime());

        STS_set(&STS_right, CLK_gethtime());
        right = echoProcessing( right, 1);
        STS_delta(&STS_right, CLK_gethtime());

        sample = _extu( right, 16, 16);
        sample = sample | _extu( left, 16, 0);

        AIC23_Write( sample);
	}

}


void tskStat(void)
{
    while (1)
    {

        TSK_sleep(10000);
        //temporarily disabled stat printf so that the audio is not choppy
        //printf( "Narocnost levy/pravy %d/%d\n", STS_left.acc / STS_left.num, STS_right.acc / STS_right.num);
        //fflush(stdout);
    }
}


// this task checks the state of DIP switches and sets effect parameters from the LookUp Tables defined bellow
void tskCheck(Arg par)
{
    static Uint16 last[4] = { 0, 0, 0, 0 };   // last DIP states

    //these const arrays are used to set the effect parameters for different DIPswitch combinations
    static const float alphaLUT[2] = { 0.3f, 0.6f };
    static const float betaLUT[2] = { 0.3f, 0.6f };
    static const Uint16 delayLUT[4] = { 50, 100, 150, 200 };

    while (1)
    {
        Uint16 sw[4];
        Uint16 i;
        for (i = 0; i < 4; i++)
        {
            sw[i] = (!DSK6416_DIP_get(i)) & 0x1;
        }

        // detect change
        if (sw[0] != last[0] || sw[1] != last[1] || sw[2] != last[2]
                || sw[3] != last[3])
        {
            // update last states and LEDs
            for (i = 0; i < 4; i++)
            {
                last[i] = sw[i];
                if (sw[i])
                    DSK6416_LED_on(i);
                else
                    DSK6416_LED_off(i);
            }

            // parameters
            float alpha = alphaLUT[sw[0]];  // DIP0-alpha
            float beta = betaLUT[sw[1]];    // DIP1-beta
            Uint16 index = (sw[2] << 1) | sw[3]; // DIP2+3-delay
            Uint16 delay = delayLUT[index];

            EchoStatus retStat = echoSetParams(delay, alpha, beta);

        }

        TSK_sleep(1000);
    }
}

void idlLive( void){
    TSK_sleep(1000);

}

/*
 *  ======== main ========
 */
Void main()
{
   int ver;

    LOG_printf(&trace, "hello world!");

	CSL_init();
	DSK6416_init();
	DSK6416_DIP_init();
	DSK6416_LED_init();

    ver = DSK6416_getVersion();
    if (1 == ver)
    {
        printf("Spuštìno v simulátoru.\n");

    }
    else
    {
        printf("Spuštìno v kitu verze %d.\n", ver);

    }

	if( AIC23_OpenCodec( &CodecCfg) < 0)
		exit(-1);

	AIC23_SetFreq( AIC23_FREQ_48KHZ);

    /* fall into DSP/BIOS idle loop */
    return;
}
