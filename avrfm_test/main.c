//
//  main.c
//  avrfm_test
//
//  Created by Tiago Rezende on 5/7/14.
//
//

#include <stdio.h>
#include "fm_synth.h"
#include "fm_command.h"
#include <stdlib.h>
#include <pthread.h>

void *command_reception_loop(void *args) {
    while (1) {
        FMCommandReceive(fgetc(stdin));
    }
    return NULL;
}

int main(int argc, const char * argv[])
{
    pthread_t thread;
    pthread_create(&thread, NULL, command_reception_loop, NULL);
    FILE * sox = popen("/usr/local/bin/sox -traw -r44100 -b8 -e signed-integer -c 2 - -tcoreaudio", "w");
    while (1) {
        FMSynth_Sample();
        fputc(fm_current_sample.left, sox);
        fputc(fm_current_sample.right, sox);
    }
    return 0;
}

