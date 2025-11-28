/*
 *  Copyright 2010 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */
/***************************************************************************/
/*                                                                         */
/*     H E L L O . C                                                       */
/*                                                                         */
/*     Basic LOG event operation from main.                                */
/*                                                                         */
/***************************************************************************/

#include <std.h>

#include <log.h>
#include <sts.h>
#include <clk.h>

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "effect.h"


#ifdef SIMULATOR

/* Simulace DIP pøepínaèù a LED diod na kitu */

#define DSK6416_DIP_get(k)      (dip[k] > 0)
#define DSK6416_LED_on(k)       (led[k] = 1)
#define DSK6416_LED_off(k)      (led[k] = 0)
#define DSK6416_LED_toggle(k)   (led[k] = led[k] ? 0 : 1)

void DSK6416_init(void)     {}
Int16 DSK6416_getVersion()  {return 1;}
void DSK6416_DIP_init(void) {}
void DSK6416_LED_init(void) {}

char led[4] = {0};
char dip[4] = {0};

Uint32 amp = 16384;

static Int16 cosine[] = {
    +16384, +16305, +16069, +15679, +15137,
    +14449, +13623, +12665, +11585, +10394,
     +9102,  +7723,  +6270,  +4756,  +3196,
     +1606,     +0,  -1606,  -3196,  -4756,
     -6270,  -7723,  -9102, -10394, -11585,
    -12665, -13623, -14449, -15137, -15679,
    -16069, -16305, -16384, -16305, -16069,
    -15679, -15137, -14449, -13623, -12665,
    -11585, -10394,  -9102,  -7723,  -6270,
     -4756,  -3196,  -1606,     +0,  +1606,
     +3196,  +4756,  +6270,  +7723,  +9102,
    +10394, +11585, +12665, +13623, +14449,
    +15137, +15679, +16069, +16305,
};

Int16 sim_input( unsigned int step)
{
  static int ind = 0;

  ind = (ind + step) & (sizeof(cosine) / sizeof( *cosine) - 1);

  return( _sshl( _smpy( amp, cosine[ind]), 2) >> 16);
}

#else

#include <dsk6416.h>
#include <dsk6416_led.h>
#include <dsk6416_dip.h>

#endif

#include "hellocfg.h"

#include "aic23.h"

static AIC23_ConfigTab CodecCfg = AIC23_DEFAULTCONFIG;

#define LENGTH	(1 << 8)

void tskProcess( void)
{
	int k;
	static Int32 sample;
	Int32 left, right;

	while( 1)
	{
		AIC23_Read( &sample);

#ifdef SIMULATOR
		sample = sim_input(3) > 0 ? 0x00003FFF : 0x0000C000;
		sample |= _extu( sample, 16, 0);

		IRQ_set(IRQ_EVT_RINT2);
		IRQ_set(IRQ_EVT_XINT2);
#endif
		left = _ext( sample, 0, 16);
		right = _ext( sample, 16, 16);

		TSK_sleep(1);
		/* simulace nároèného zpracování */
		if( !DSK6416_DIP_get(3) && rand() < 1000)
			for( k = 0; k < 50000; k++)
				;

        STS_set(&STS_left, CLK_gethtime());
        left = echoProcessing( right, 100, 0.5f, 0.5f);
        STS_delta( &STS_left, CLK_gethtime());

        STS_set(&STS_right, CLK_gethtime());
        right = right;
        STS_delta(&STS_right, CLK_gethtime());

        sample = _extu( right, 16, 16);
        sample = sample | _extu( left, 16, 0);

        AIC23_Write( sample);
	}

	AIC23_CloseCodec();
}


void tskStat( void)
{
    while( 1){
#ifdef SIMULATOR
        TSK_sleep(100);
#else
        TSK_sleep(10000);
#endif
        printf( "Narocnost levy/pravy %d/%d\n", STS_left.acc / STS_left.num, STS_right.acc / STS_right.num);
        fflush(stdout);
    }
}


void tskCheck( Arg par)
{
	static int last[2];

	while( 1){
		last[par] = !DSK6416_DIP_get(par);
		if( last[par]){
			DSK6416_LED_on(par);
		} else
			DSK6416_LED_off(par);

		switch(par){
		case 0:
			AIC23_Mute(last[par]);
			break;
		case 1:
			AIC23_Loopback(last[par]);
			break;
		}

#ifdef SIMULATOR
       IRQ_set( IRQ_EVT_RINT2);
       IRQ_set( IRQ_EVT_XINT2);
#endif

		while( last[par] == !DSK6416_DIP_get(par))
			TSK_sleep(1000);
	}
}

void idlLive( void){
	static int count = 0;

	if( 500000 > ++count)
		return;

	DSK6416_LED_toggle(3);
	count = 0;
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
    if( 1 == ver){
        printf( "Spuštìno v simulátoru.\n");
#ifndef SIMULATOR
        printf( "V simulátoru není audio kodek a proto verze pro kit nebude fungovat.\n");
        exit(-1);
#endif
    } else {
        printf( "Spuštìno v kitu verze %d.\n", ver);
#ifdef SIMULATOR
        printf( "Verzi pro simulátor nelze spouštìt na kitu.\n");
        exit( -1);
#endif
    }

	if( AIC23_OpenCodec( &CodecCfg) < 0)
		exit(-1);

	AIC23_SetFreq( AIC23_FREQ_44KHZ);

    /* fall into DSP/BIOS idle loop */
    return;
}
