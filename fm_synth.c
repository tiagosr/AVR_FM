/*
 *  fm_synth.c
 *  AVR_FM
 *
 *  Created by Tiago Rezende on 4/14/11.
 *  Copyright 2011 Pixel of View. All rights reserved.
 *
 */

#include "fm_synth.h"
#include <avr/pgmspace.h>

FMChannel fm_channels[FM_CHANNEL_COUNT];
FMSample fm_current_sample;
uint8_t fm_master_volume;

/*
 * python program used to make the table below:

import math
sin_table = []
for x in xrange(256):
    sin_table += [int(math.sin((x/256.0)*math.pi*2)*128 - 0.5) ,]
print sin_table

 */
static const int8_t sin_table[256] PROGMEM = {
	0, 2, 5, 8, 12, 15, 18, 21,
	24, 27, 30, 33, 36, 39, 42, 45,
	48, 51, 54, 57, 59, 62, 65, 67,
	70, 73, 75, 78, 80, 83, 85, 87,
	90, 92, 94, 96, 98, 100, 102, 104,
	105, 107, 109, 110, 112, 113, 115, 116,
	117, 118, 120, 121, 121, 122, 123, 124,
	125, 125, 126, 126, 126, 127, 127, 127,
	127, 127, 127, 127, 126, 126, 126, 125,
	125, 124, 123, 122, 121, 121, 120, 118,
	117, 116, 115, 113, 112, 110, 109, 107,
	105, 104, 102, 100, 98, 96, 94, 92,
	90, 87, 85, 83, 80, 78, 75, 73,
	70, 67, 65, 62, 59, 57, 54, 51,
	48, 45, 42, 39, 36, 33, 30, 27,
	24, 21, 18, 15, 12, 8, 5, 2,
	0, -3, -6, -9, -13, -16, -19, -22,
	-25, -28, -31, -34, -37, -40, -43, -46,
	-49, -52, -55, -58, -60, -63, -66, -68,
	-71, -74, -76, -79, -81, -84, -86, -88,
	-91, -93, -95, -97, -99, -101, -103, -105,
	-106, -108, -110, -111, -113, -114, -116, -117,
	-118, -119, -121, -122, -122, -123, -124, -125,
	-126, -126, -127, -127, -127, -128, -128, -128,
	-128, -128, -128, -128, -127, -127, -127, -126,
	-126, -125, -124, -123, -122, -122, -121, -119,
	-118, -117, -116, -114, -113, -111, -110, -108,
	-106, -105, -103, -101, -99, -97, -95, -93,
	-91, -88, -86, -84, -81, -79, -76, -74,
	-71, -68, -66, -63, -60, -58, -55, -52,
	-49, -46, -43, -40, -37, -34, -31, -28,
	-25, -22, -19, -16, -13, -9, -6, -3
};

int8_t FMOscillator_Sample(FMOscillator *osc, uint16_t phase)
{
	switch (osc->wave_type) {
		case FMWave_Disabled:
			return 0;
		case FMWave_Sine:
			return pgm_read_byte(&(sin_table[(phase&0xff00)>>8]));
		case FMWave_Square:
			return (phase&0x8000)?-0x80:0x7f;
		case FMWave_Triangle:
			return ((phase&0x8000)?(0x7fff-phase):(phase-0x7fff))>>7;
		case FMWave_Sawtooth:
			return ((phase&0xff00)>>8)-128;
		default:
			return 0;
	}
}

int8_t FMEnvelope_Sample(FMEnvelope *env, uint16_t frequency, uint8_t note_on)
{
	uint16_t freq_rate_change = (frequency*env->rate_change)>>8;
	switch (env->stage) {
		case 0: // stopped
			if (note_on) {
				env->stage = 1; // set to attack
				// fall through to attack phase
			} else {
                break;
			}
		case 1: // attack
			env->level += env->level + env->attack_rate + ((freq_rate_change *env->attack_rate)>>8);
			if (env->level >= ((env->total_level * note_on)>>8)) { // reached total level
				env->level = (env->total_level * note_on)>>8;
				env->stage = 2; // set to decay
			}
			break;
		case 2: // decay
			env->level -= env->decay_rate + ((freq_rate_change * env->decay_rate)>>8);
			if (env->level <= ((env->sustain_level * note_on)>>8)) { // note reached sustain level
				env->level = (env->sustain_level *note_on) >> 8;
				env->stage = 3; // set to sustain
			}
			break;
		case 3: // sustain & release
			if (note_on) { // sustain while note is not released
				env->level -= env->sustain_decay_rate + ((freq_rate_change * env->sustain_decay_rate)>>8);
			} else {
				env->level -= env->release_rate + ((freq_rate_change * env->release_rate)>>8);
			}
			if (env->level <= 0) { // end envelope
				env->level = 0; // zero level
				env->stage = 4; // set stage to debounce
			}
			break;
		case 4: // debounce
		default:
			if (!note_on) { // note released
				env->stage = 0; // set to initial stage
			}
			break;
	}
	return env->level;
}

void FMEnvelope_Reset(FMEnvelope *env)
{
	env->level = 0;
	env->stage = 4;
}

void FMOperator_Reset(FMOperator *op)
{
	op->phase = 0;
}
void FMOperator_Sample(FMOperator *op, int8_t octave, uint8_t note, uint8_t note_on, int8_t offset)
{
	uint16_t frequency = ((note+(op->detune_multiplier>>4))*(1<<octave));
	op->phase += frequency * (op->detune_multiplier & 0xf);

	uint16_t phase2 = op->phase+((op->offset_influence*offset)>>8);

	op->value = (((FMEnvelope_Sample(&op->envelope, frequency, note_on) *
				   FMOscillator_Sample(&op->oscillator, phase2)) >> 8) *
				 op->index) >> 8;
}



typedef struct fm_algorithm_config {
	uint8_t stage_map[FM_OPERATOR_COUNT+1];
} fm_algorithm_config;


#define algo_slot(a, b, c, d) (((a)<<3)|((b)<<2)|((c)<<1)|(d))
static const fm_algorithm_config algorithm_configs[] PROGMEM = {
	{
		{	/*
			 * algorithm 0:
			 *
			 * s0 <- old s0
			 * |
			 * s1
			 * |
			 * s2
			 * |
			 * s3
			 * |
			 * out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,1,0,0),	// s1 => s2
			algo_slot(0,0,1,0),	// s2 => s3
			algo_slot(0,0,0,1)		// s3 => out
		}
	}, {
	    {	/*
			 * algorithm 1:
			 *
			 * s0 <- old s0
			 * |
			 * s1
			 * |
			 * s2 + s3
			 *    |
			 *   out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,1,0,0),	// s1 => s2
			algo_slot(0,0,0,0),			// xx => s3
			algo_slot(0,0,1,1)	// s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 2:
			 *
			 * s0 <- old s0
			 * |
			 * s1 + s2 + s3
			 *       |
			 *      out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,0,0,0),			// xx => s2
			algo_slot(0,0,0,0),			// xx => s3
			algo_slot(0,1,1,1)	// s1 + s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 3:
			 *
			 * s0 <- old s0
			 * |
			 * |    s2
			 * |    |
			 * s1 + s3
			 *    |
			 *   out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,0,0,0),	// xx => s2
			algo_slot(0,0,1,0),	// s2 => s3
			algo_slot(0,1,0,1)	// s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 4:
			 *
			 * s0 <- old s0
			 * |
			 * |    s2 <- old s2
			 * |    |
			 * s1 + s3
			 *    |
			 *   out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,0,1,0),	// s2 feedback => s2
			algo_slot(0,0,1,0),	// s2 => s3
			algo_slot(0,1,0,1)	// s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 5:
			 *
			 *    s0 <- old s0
			 *    |
			 *    s1
			 *    |
			 * +--+--+
			 * |     |
			 * s2 + s3
			 *    |
			 *   out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(0,1,0,0),	// s1 => s2
			algo_slot(0,1,0,0),	// s1 => s3
			algo_slot(0,0,1,1)	// s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 6:
			 *
			 *   s0 <- old s0
			 *   |
			 * +-+--+
			 * |    |
			 * s1 + s2 + s3
			 *      |
			 *     out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(1,0,0,0),	// s0 => s2
			algo_slot(0,0,0,0),	// xx => s3
			algo_slot(0,1,1,1)	// s2 + s3 => out
		}
	}, {
		{	/*
			 * algorithm 7:
			 *
			 *      s0 <- old s0
			 *      |
			 * +----+----+
			 * |    |    |
			 * s1 + s2 + s3
			 *      |
			 *     out
			 *
			 */
			algo_slot(1,0,0,0),	// s0 feedback => s0
			algo_slot(1,0,0,0),	// s0 => s1
			algo_slot(1,0,0,0),	// s0 => s2
			algo_slot(1,0,0,0),	// s0 => s3
			algo_slot(0,1,1,1)	// s1+ s2 + s3 => out
		}
	}
};

FMSample FMChannel_Sample(FMChannel *channel) {

	uint8_t vol_left = (127-channel->panning);
	uint8_t vol_right = (127+channel->panning);
	if (vol_right>127) vol_right = 127;
	if (vol_left>127) vol_left = 127;
	int8_t accum;

	uint8_t i = 0;
	uint8_t j;
	for (; i<FM_OPERATOR_COUNT; i++) {
		j = 0;
		accum = 0;
		for (; j<FM_OPERATOR_COUNT; j++) {
			uint8_t flag = (pgm_read_byte(&(algorithm_configs[channel->algorithm_octave >> 4].stage_map[i]))&(1<<j));
			if (flag) accum += channel->operators[j].value;
		}
		FMOperator_Sample(&(channel->operators[i]), channel->algorithm_octave & 0xf, channel->note, channel->note_on, accum);
	}
	accum = 0;
	for (j=0; j<FM_OPERATOR_COUNT; j++) {
		uint8_t flag = (pgm_read_byte(&(algorithm_configs[channel->algorithm_octave >> 4].stage_map[i]))&(1<<j));
        if (flag) accum += channel->operators[j].value;
	}
	FMSample sample = {accum*vol_left,accum*vol_right};
	return sample;
}

void FMSynth_Sample() {
	fm_current_sample.left = fm_current_sample.right = 0;
	FMSample sample;
	uint8_t i = 0;
	for (; i<FM_CHANNEL_COUNT; i++) {
		sample = FMChannel_Sample(&fm_channels[i]);
		fm_current_sample.left += sample.left >> 3; // sample/8
		fm_current_sample.right += sample.right >> 3; // sample/8
	}
	fm_current_sample.left = (fm_current_sample.left * fm_master_volume)>>8;
	fm_current_sample.right = (fm_current_sample.right * fm_master_volume)>>8;
}


