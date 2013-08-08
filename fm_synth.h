/*
 *  fm_synth.h
 *  AVR_FM
 *
 *  Created by Tiago Rezende on 4/14/11.
 *  Copyright 2011 Pixel of View. All rights reserved.
 *
 */

#ifndef _fm_synth_H_
#define _fm_synth_H_

#include <avr/io.h>
#define FM_OPERATOR_COUNT 4
#define FM_CHANNEL_COUNT 6

/* Sample rate is 8Mhz/256 */
#define FM_SAMPLE_RATE 31250

#define FM_OUTPUT_PWM
#define FM_OUTPUT_LEFT	PB0
#define FM_OUTPUT_RIGHT	PB1

typedef enum {
	FMWave_Disabled = 0,
	FMWave_Sine,
	FMWave_Square,
	FMWave_Triangle,
	FMWave_Sawtooth,
	FMWave_Noise
} FMWaveType;

typedef struct FMOscillator {
	FMWaveType wave_type;	// 1
	uint8_t flags;			// 1
} FMOscillator;				// = 2

int8_t FMOscillator_Sample(FMOscillator *osc, uint16_t phase);

typedef struct FMEnvelope {
	int8_t attack_rate;			// 1
	int8_t total_level;			// 1
	int8_t am_decay_rate;			// 1
	int8_t sustain_level;		// 1
	int8_t sustain_decay_rate;	// 1
	int8_t release_rate;		// 1

	uint8_t rate_change;		// 1
	int8_t stage;				// 1
	int8_t level;				// 1
} FMEnvelope;	// = 8

int8_t FMEnvelope_Sample(FMEnvelope *env, uint16_t frequency, uint8_t note_on);

typedef struct FMOperator {
	FMOscillator oscillator;	// 2
	FMEnvelope envelope;		// 8
	int8_t detune_multiplier;	// 1
	int8_t index;				// 1
	int8_t offset_influence;	// 1
	int16_t phase;				// 2
	int8_t value;				// 1
} FMOperator;					// = 16

void FMOperator_Reset(FMOperator *op);
void FMOperator_Sample(FMOperator *op, int8_t octave, uint8_t note, uint8_t note_on, int8_t offset);

typedef struct FM_Channel {
	FMOperator operators[FM_OPERATOR_COUNT];	// 16*4 = 64
	int8_t algorithm_octave;	// 1
	uint8_t note;				// 1 // test if enough resolution. another solution might be a lookup table with values of 2^(x/12)
	uint8_t note_on;			// 1
	int8_t panning;				// 1
} FMChannel;					// = 68

typedef struct FMSample {
	int8_t left, right;
} FMSample;

FMSample FMChannel_Sample(FMChannel *channel);

extern FMChannel fm_channels[FM_CHANNEL_COUNT];	// 73*6 = 438 bytes =)
extern uint8_t fm_master_volume;				// 1
extern FMSample fm_current_sample;				// 2
void FMSynth_Sample();

// total data: 441 bytes

#endif //ndef _fm_synth_H_
