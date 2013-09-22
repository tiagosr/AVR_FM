/*
 *  main.c
 *  AVR_FM
 *
 *  Created by Tiago Rezende on 8/4/13.
 *  Copyright 2011 Pixel of View. All rights reserved.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "fm_synth.h"

ISR(TIMER1_OVF_vect) {
    OCR1A = fm_current_sample.left;
    OCR2A = fm_current_sample.right;
    FMSynth_Sample(); // sample after so it doesn't suffer aliasing problems
}

int main(void)
{
    TIMSK1 = (1<<TOIE1);
    TCNT0 = 0x00;
    TCCR1B |= (1<<WGM12)|(1<<WGM13)| 1; // clkio
    DDRB |= (1<<DDB1)|(1<<DDB2);
    sei(); // enable interrupts
    while(1) {
    }

    return 0;
}
