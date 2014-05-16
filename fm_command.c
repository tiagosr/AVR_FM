//
//  fm_command.c
//  AVR_FM
//
//  Created by Tiago Rezende on 5/15/14.
//
//

#include "fm_synth.h"
#include "fm_command.h"

#ifdef BUILD_FOR_AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM
static int8_t pgm_read_byte(const void *ptr) {
    return (*((int8_t*)ptr));
}
#endif

#define scrBegin         static int scrLine = 0; switch(scrLine) { case 0:;
#define scrFinish(z)     } return (z)
#define scrFinishV       } return

#define scrYield(z)     \
do {\
scrLine=__LINE__;\
return (z); case __LINE__:;\
} while (0)
#define scrYieldV       \
do {\
scrLine=__LINE__;\
return; case __LINE__:;\
} while (0)

static uint8_t channel_num = 0, op_num = 0;

static inline uint8_t FMSetChannel(uint8_t data) {
    if (data >= FM_CHANNEL_COUNT) {
        return 1;
    }
    channel_num = data;
    return 0;
}
static inline uint8_t FMSetOperator(uint8_t data) {
    if (data >= FM_OPERATOR_COUNT) {
        return 1;
    }
    op_num = data;
    return 0;
}

// [int((x/12.0)*256.0)&0xff for x in xrange(128)]
static const uint8_t midi_notes[128] PROGMEM = {
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149, 170, 192, 213, 234,
    0, 21, 42, 64, 85, 106, 128, 149
};
// [(int((x/12.0)*256.0)&0xff00) >> 8 for x in xrange(128)]
static const uint8_t midi_octaves[128] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    10, 10, 10, 10, 10, 10, 10, 10
};

uint8_t FMCommandReceive(uint8_t command) {
    scrBegin;
    static uint8_t octave = 0, bytes_remaining = 0, current_byte = 0;
    while (1) {
        if (command == FM_SetChan) {
            scrYield(1);
            FMSetChannel(command);
        } else if (command == FM_SetOper) {
            scrYield(1);
            FMSetOperator(command);
        } else if (command == FM_MIDINoteOn) {
            scrYield(1);
            fm_channels[channel_num].note = pgm_read_byte(&midi_notes[command>>1]);
            fm_channels[channel_num].octave = pgm_read_byte(&midi_octaves[command>>1]);
            fm_channels[channel_num].note_on = 1;
        } else if (command == FM_NoteOn) {
            scrYield(2);
            octave = command;
            scrYield(1);
            fm_channels[channel_num].octave = octave;
            fm_channels[channel_num].note = command;
            fm_channels[channel_num].note_on = 1;
        } else if (command == FM_NoteOff) {
            fm_channels[channel_num].note_on = 0;
            scrYield(0);
        } else if (command == FM_SetNote) {
            scrYield(2);
            octave = command;
            scrYield(1);
            fm_channels[channel_num].octave = octave;
            fm_channels[channel_num].note = command;
        } else if (command == FM_SetRaw) {
            bytes_remaining = sizeof(FMChannel);
            while (bytes_remaining) {
                scrYield(bytes_remaining);
                ((uint8_t *)((void *)fm_channels))[current_byte] = command;
                current_byte++; bytes_remaining--;
            }
            scrYield(0);
        } else {
            scrYield(0);
        }
    }
    scrFinish(0);
}

