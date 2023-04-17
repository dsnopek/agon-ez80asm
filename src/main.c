#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "assemble.h"
#include "label.h"
#include "./stdint.h"
#include "macro.h"
#include "mos-interface.h"
#include "mos_posix.h"
#include "malloc.h"
#include "io.h"

int main(int argc, char *argv[])
{
    // Init posix compatibility
    mos_posix_init();    

    if(argc < 2){
        printf("Usage: asm <filename> [-l]\n\r");
        return 0;
    }

    list_enabled = ((argc == 3) && (strcmp(argv[2], "-l") == 0));     
    consolelist_enabled = false;

    prepare_filenames(argv[1]);
    if(!openfiles()) return 0;

    // Initialization
    outputBufferInit();
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();
    initMacros();
    init_agon_malloc();
    
    // Assemble input to output
    assemble();
    if(global_errors) {
        mos_del(filename[FILE_OUTPUT]);
        printf("Error in input\r\n");
    }
    else printf("Done\r\n");

    outputBufferFlush();
    closeAllFiles();   
    return 0;
}