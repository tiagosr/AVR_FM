/*
 *  fm_synth.c
 *  AVR_FM
 *
 *  Created by Tiago Rezende on 4/14/11.
 *  Copyright 2011 Pixel of View. All rights reserved.
 *
 */

#include "fm_synth.h"
#ifdef BUILD_FOR_AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM  
static int8_t pgm_read_byte(const void *ptr) {
    return (*((int8_t*)ptr));
}
#endif

FMChannel fm_channels[FM_CHANNEL_COUNT];
FMSample fm_current_sample;
uint8_t fm_master_volume;

/*
 * python program used to make the table below:

import math
print [int(math.sin((x/256.0)*math.pi*2)*128 - 0.5) for x in xrange(256)]

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

// [int((math.pow(2, (x/512.0))-1.0)*256) for x in xrange(512)]
static const uint8_t scale_table[512] PROGMEM = {
    0, 0, 0, 1, 1, 1, 2, 2,
    2, 3, 3, 3, 4, 4, 4, 5,
    5, 5, 6, 6, 7, 7, 7, 8,
    8, 8, 9, 9, 9, 10, 10, 10,
    11, 11, 12, 12, 12, 13, 13, 13,
    14, 14, 14, 15, 15, 16, 16, 16,
    17, 17, 17, 18, 18, 19, 19, 19,
    20, 20, 20, 21, 21, 22, 22, 22,
    23, 23, 23, 24, 24, 25, 25, 25,
    26, 26, 26, 27, 27, 28, 28, 28,
    29, 29, 30, 30, 30, 31, 31, 31,
    32, 32, 33, 33, 33, 34, 34, 35,
    35, 35, 36, 36, 37, 37, 37, 38,
    38, 39, 39, 39, 40, 40, 41, 41,
    41, 42, 42, 43, 43, 43, 44, 44,
    45, 45, 45, 46, 46, 47, 47, 48,
    48, 48, 49, 49, 50, 50, 50, 51,
    51, 52, 52, 53, 53, 53, 54, 54,
    55, 55, 55, 56, 56, 57, 57, 58,
    58, 58, 59, 59, 60, 60, 61, 61,
    61, 62, 62, 63, 63, 64, 64, 64,
    65, 65, 66, 66, 67, 67, 67, 68,
    68, 69, 69, 70, 70, 71, 71, 71,
    72, 72, 73, 73, 74, 74, 75, 75,
    75, 76, 76, 77, 77, 78, 78, 79,
    79, 80, 80, 80, 81, 81, 82, 82,
    83, 83, 84, 84, 85, 85, 86, 86,
    86, 87, 87, 88, 88, 89, 89, 90,
    90, 91, 91, 92, 92, 93, 93, 93,
    94, 94, 95, 95, 96, 96, 97, 97,
    98, 98, 99, 99, 100, 100, 101, 101,
    102, 102, 103, 103, 104, 104, 105, 105,
    106, 106, 107, 107, 108, 108, 108, 109,
    109, 110, 110, 111, 111, 112, 112, 113,
    113, 114, 114, 115, 115, 116, 116, 117,
    117, 118, 119, 119, 120, 120, 121, 121,
    122, 122, 123, 123, 124, 124, 125, 125,
    126, 126, 127, 127, 128, 128, 129, 129,
    130, 130, 131, 131, 132, 132, 133, 134,
    134, 135, 135, 136, 136, 137, 137, 138,
    138, 139, 139, 140, 140, 141, 142, 142,
    143, 143, 144, 144, 145, 145, 146, 146,
    147, 147, 148, 149, 149, 150, 150, 151,
    151, 152, 152, 153, 154, 154, 155, 155,
    156, 156, 157, 157, 158, 159, 159, 160,
    160, 161, 161, 162, 163, 163, 164, 164,
    165, 165, 166, 167, 167, 168, 168, 169,
    169, 170, 171, 171, 172, 172, 173, 173,
    174, 175, 175, 176, 176, 177, 178, 178,
    179, 179, 180, 180, 181, 182, 182, 183,
    183, 184, 185, 185, 186, 186, 187, 188,
    188, 189, 189, 190, 191, 191, 192, 192,
    193, 194, 194, 195, 196, 196, 197, 197,
    198, 199, 199, 200, 200, 201, 202, 202,
    203, 204, 204, 205, 205, 206, 207, 207,
    208, 209, 209, 210, 210, 211, 212, 212,
    213, 214, 214, 215, 216, 216, 217, 217,
    218, 219, 219, 220, 221, 221, 222, 223,
    223, 224, 225, 225, 226, 227, 227, 228,
    229, 229, 230, 230, 231, 232, 232, 233,
    234, 234, 235, 236, 236, 237, 238, 238,
    239, 240, 240, 241, 242, 242, 243, 244,
    245, 245, 246, 247, 247, 248, 249, 249,
    250, 251, 251, 252, 253, 253, 254, 255
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
    uint8_t rate_scaling_factor = 4-(env->rate_scaling__attack_rate >> 5);
    uint8_t sustain_level = (env->sustain_level__release_rate & 0xf0);
    sustain_level += sustain_level>>4;

    uint16_t rate_scaling = (frequency&0x7a0)>>rate_scaling_factor;
	switch (env->stage) {
		case 0: // stopped
			if (note_on) {
				env->stage = 1; // set to attack
				// fall through to attack phase
			} else {
                break;
			}
		case 1: // attack
		    {
		        uint8_t attack_rate = ((env->rate_scaling__attack_rate &0x1f)+1) << 3;
                env->level += attack_rate + attack_rate*rate_scaling;
                if (env->level >= ((env->total_level * note_on)>>8)) { // reached total level
                    env->level = (env->total_level * note_on)>>8;
                    env->stage = 2; // set to decay
                }
		    }
			break;
		case 2: // decay
		    {
		        uint8_t decay_rate = (env->am__decay_rate & 0x1f) << 3;
		        uint8_t sustain_level = (env->sustain_level__release_rate &0xf0);
		        sustain_level += sustain_level >> 4;
                env->level -= decay_rate + ((decay_rate * rate_scaling)>>8);
                if (env->level <= ((sustain_level * note_on)>>8)) { // note reached sustain level
                    env->level = (sustain_level * note_on) >> 8;
                    env->stage = 3; // set to sustain
                }
			}
			break;
		case 3: // sustain & release
		    {
		        if (note_on) { // sustain while note is not released
                    env->level -= env->sustain_decay_rate + ((rate_scaling * env->sustain_decay_rate)>>8);
                } else {
                    uint8_t release_rate = (env->sustain_level__release_rate & 0xf);
                    release_rate += release_rate << 4;
		            env->level -= release_rate + ((rate_scaling * release_rate)>>8);
                }
                if (env->level <= 0) { // end envelope
                    env->level = 0; // zero level
                    env->stage = 4; // set stage to debounce
                }
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
    uint8_t envelope_sample = FMEnvelope_Sample(&op->envelope, octave << 8 | note, note_on);
    uint16_t frequency = ((pgm_read_byte(&scale_table[note])+(op->detune_multiplier>>4))*(1<<octave));
	op->phase += frequency * ((op->detune_multiplier & 0xf) + 1);

	uint16_t phase2 = op->phase+((op->offset_influence*offset)>>8);

	op->value = (((envelope_sample *
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
			uint8_t flag = (pgm_read_byte(&(algorithm_configs[channel->algorithm].stage_map[i]))&(1<<j));
			if (flag) accum += channel->operators[j].value;
		}
		FMOperator_Sample(&(channel->operators[i]),
                          channel->octave,
                          channel->note,
                          channel->note_on, accum);
	}
	accum = 0;
	for (j=0; j<FM_OPERATOR_COUNT; j++) {
		uint8_t flag = (pgm_read_byte(&(algorithm_configs[channel->algorithm].stage_map[i]))&(1<<j));
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


