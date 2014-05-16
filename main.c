/*
 *  main.c
 *  AVR_FM
 *
 *  Created by Tiago Rezende on 8/4/13.
 *  Copyright 2011 Pixel of View. All rights reserved.
 *
 */

#ifdef BUILD_FOR_AVR
#include <avr/io.h>
#include <avr/interrupt.h>
#else
#include <stdint.h>
#endif



#include "fm_synth.h"
#include "fm_command.h"

#ifdef BUILD_FOR_AVR
ISR(TIMER1_OVF_vect) {
    OCR1A = fm_current_sample.left;
    OCR2A = fm_current_sample.right;
    FMSynth_Sample(); // sample after so it doesn't suffer aliasing problems
}

static void FMSampleOutputInit() {
    TIMSK1 = (1<<TOIE1);
    TCNT0 = 0x00;
    TCCR1B |= (1<<WGM12)|(1<<WGM13)| 1; // clkio
    DDRB |= (1<<DDB1)|(1<<DDB2);
}


ISR(SPI_STC_vect) {
    SPDR = FMCommandReceive(SPDR);
}

#define DD_MOSI 5
#define DD_MISO 6
#define DD_SCK 7
#define DDR_SPI DDRB

static void FMCommandSPIInputInit() {
    DDR_SPI |= (1<<DD_MISO);
    // set SPI enable, spi clock polarity ,  spi interrupts enable
	SPCR = (1<<SPE)|(1<<CPOL)|(1<<SPIE);
}

int main(void)
{
    FMSampleOutputInit();
    FMCommandSPIInputInit();
    sei(); // enable interrupts
    while(1) {
    }

    return 0;
}
#else



int main(int argc, char *argv) {
    
}
#endif