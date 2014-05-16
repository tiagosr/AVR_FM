//
//  fm_command.h
//  AVR_FM
//
//  Created by Tiago Rezende on 5/15/14.
//
//

#ifndef AVR_FM_fm_command_h
#define AVR_FM_fm_command_h
#include "fm_synth.h"
#include <inttypes.h>

typedef enum {
    FM_Idle,
    
    FM_SetChan,
    FM_SetOper,
    FM_SetNote,
    
    FM_NoteOn,
    FM_NoteOff,
    FM_SetRaw,
    
    FM_OscSetData,
    
    FM_EnvSetData,
    FM_EnvSetRateScaling_AttackRate,
    FM_EnvSetLevel,
    FM_EnvSetTotalLevel,
    FM_EnvSetAmplitudeMod_DecayRate,
	FM_EnvSetSustainLevel_ReleaseRate,
	FM_EnvSetSustainDecay,

    
    FM_OperSetData,
    FM_OperSetDetuneMultiplier,
    FM_OperSetIndex,
    FM_OperSetOffsetInfluence,
    
    FM_ChanSetData,
    FM_ChanSetAlgorithm_Octave,
    FM_ChanSetNote,
    FM_ChanSetPan,
    
    FM_MIDINoteOn,
    
} FMCommand;

uint8_t FMCommandReceive(uint8_t command);


#endif
