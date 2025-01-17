#include <stdio.h>
#include <string.h>
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"
#include "./stdint.h"
#include "filestack.h"
#include "mos-interface.h"
#include "io.h"

// memory for anonymous labels
anonymouslabeltype an_prev;
anonymouslabeltype an_next;
label an_return;

char localLabelBuffer[LOCAL_LABEL_BUFFERSIZE];
uint16_t localLabelBufferIndex;

// tables
label* globalLabelTable[GLOBAL_LABEL_TABLE_SIZE]; // hash table
uint16_t globalLabelCounter;
label localLabelTable[LOCAL_LABEL_TABLE_SIZE]; // indexed table
uint16_t localLabelCounter;

void printLocalLabelTable(void) {
    int i;
    printf("\r\nLocal table (%d entries):\n\r",localLabelCounter);
    for(i = 0; i < localLabelCounter; i++) printf("%08x - %s\n\r", localLabelTable[i].address, localLabelTable[i].name);
}

void printGlobalLabelTable(void) {
    int i;
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++) {
        if(globalLabelTable[i]) {
            printf("%s - %x\n", globalLabelTable[i]->name, globalLabelTable[i]->address);
        }
    }
}
uint16_t getGlobalLabelCount(void) {
    return globalLabelCounter;
}
uint16_t getLocalLabelCount(void) {
    return localLabelCounter;
}
void initGlobalLabelTable(void) {
    int i;

    globalLabelCounter = 0;
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++){
        globalLabelTable[i] = NULL;
    }
}

void initAnonymousLabelTable(void) {
    an_prev.defined = false;
    an_next.defined = false;
    an_return.name = NULL;
}

void initLocalLabelTable(void) {
    int i;

    localLabelBufferIndex = 0;
    localLabelCounter = 0;
    for(i = 0; i < LOCAL_LABEL_TABLE_SIZE; i++) {
        localLabelTable[i].name = NULL;
    }
}

void clearLocalLabels(void) {
    localLabelBufferIndex = 0;
    localLabelCounter = 0;
}

int findLocalLabelIndex(char *key) {
    int base = 0;
	int lim, cmp;
    int p;

	for (lim = localLabelCounter; lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcmp(key,localLabelTable[p].name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (LOCAL_LABEL_TABLE_SIZE);
}

label * findLocalLabel(char *key){
    int p = findLocalLabelIndex(key);
    if(p < LOCAL_LABEL_TABLE_SIZE) return &localLabelTable[p];
    else return NULL;
}

void writeLocalLabels(void) {
    int i;
    uint16_t delta;
    // number of bytes in the string buffer
    //agon_fwrite(&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, FILE_LOCAL_LABELS);
    mos_fwrite(filehandle[FILE_LOCAL_LABELS], (char *)&localLabelBufferIndex, sizeof(localLabelBufferIndex));
    // the actual bytes from the string buffer
    //if(localLabelBufferIndex) agon_fwrite(localLabelBuffer, localLabelBufferIndex, 1, FILE_LOCAL_LABELS);
    if(localLabelBufferIndex) 
        mos_fwrite(filehandle[FILE_LOCAL_LABELS], (char *)localLabelBuffer, localLabelBufferIndex);
    
    // the number of labels
    //agon_fwrite(&localLabelCounter, sizeof(localLabelCounter), 1, FILE_LOCAL_LABELS);
    mos_fwrite(filehandle[FILE_LOCAL_LABELS], (char *)&localLabelCounter, sizeof(localLabelCounter));
    for(i = 0; i < localLabelCounter; i++) {
        delta = localLabelTable[i].name - localLabelBuffer;
        //agon_fwrite(&delta, sizeof(delta), 1, FILE_LOCAL_LABELS);
        mos_fwrite(filehandle[FILE_LOCAL_LABELS], (char *)&delta, sizeof(delta));
        //agon_fwrite(&localLabelTable[i].address, 4, 1, FILE_LOCAL_LABELS);
        mos_fwrite(filehandle[FILE_LOCAL_LABELS], (char *)&localLabelTable[i].address, 4);
    }
}

void readLocalLabels(void) {
    int i;
    uint16_t delta;

    //agon_fread(&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, FILE_LOCAL_LABELS);
    mos_fread(filehandle[FILE_LOCAL_LABELS], (char*)&localLabelBufferIndex, sizeof(localLabelBufferIndex));
    //if(localLabelBufferIndex) agon_fread(&localLabelBuffer, localLabelBufferIndex, 1, FILE_LOCAL_LABELS);
    if(localLabelBufferIndex) 
        mos_fread(filehandle[FILE_LOCAL_LABELS], (char*)&localLabelBuffer, localLabelBufferIndex);
    //agon_fread(&localLabelCounter, sizeof(localLabelCounter), 1, FILE_LOCAL_LABELS);
    mos_fread(filehandle[FILE_LOCAL_LABELS], (char*)&localLabelCounter, sizeof(localLabelCounter));
    for(i = 0; i < localLabelCounter; i++) {
        //agon_fread(&delta, sizeof(delta), 1, FILE_LOCAL_LABELS);
        mos_fread(filehandle[FILE_LOCAL_LABELS], (char*)&delta, sizeof(delta));
        //agon_fread(&localLabelTable[i].address, 4, 1, FILE_LOCAL_LABELS);
        mos_fread(filehandle[FILE_LOCAL_LABELS], (char*)&localLabelTable[i].address, 4);        
        localLabelTable[i].name = localLabelBuffer + delta;
    }
}

void writeAnonymousLabel(int24_t address) {
    uint8_t scope;

    scope = filestackCount();
    //agon_fwrite(&address, sizeof(address), 1, FILE_ANONYMOUS_LABELS);
    mos_fwrite(filehandle[FILE_ANONYMOUS_LABELS], (char*)&address, sizeof(address));
    //agon_fwrite(&scope, sizeof(scope), 1, FILE_ANONYMOUS_LABELS);
    mos_fwrite(filehandle[FILE_ANONYMOUS_LABELS], (char*)&scope, sizeof(scope));
}

void readAnonymousLabel(void) {
    int24_t address;
    uint8_t scope;

//    if(agon_fread(&address, sizeof(address), 1, FILE_ANONYMOUS_LABELS)) {
    if(mos_fread(filehandle[FILE_ANONYMOUS_LABELS], (char*)&address, sizeof(address))) {
        //agon_fread(&scope, sizeof(bool), 1, FILE_ANONYMOUS_LABELS);
        mos_fread(filehandle[FILE_ANONYMOUS_LABELS], (char*)&scope, sizeof(bool));
        if(an_next.defined) {
            an_prev.address = an_next.address;
            an_prev.scope = an_next.scope;
            an_prev.defined = true;
        }
        an_next.address = address;
        an_next.scope = scope;            
        an_next.defined = true;
    }
    else { // last label already read
        an_prev.address = an_next.address;
        an_prev.scope = an_next.scope;
        an_prev.defined = true;
        an_next.defined = false;
    }
}

bool insertLocalLabel(char *labelname, int24_t address) {
    int len,i;
    int p;
    char *ptr;
    char *old_name;
    int24_t old_address;

    if(labelname[1] == 0) {
        error(message[ERROR_INVALIDLABEL]);
        return false;
    }
    p = findLocalLabelIndex(labelname);

    if(p >= LOCAL_LABEL_TABLE_SIZE) {
        if(localLabelCounter < LOCAL_LABEL_TABLE_SIZE) {
            len = strlen(labelname);
            // check space first
            if((localLabelBufferIndex + len + 1) > LOCAL_LABEL_BUFFERSIZE -1)
                return false; // no more space in buffer
            // allocate space in buffer for string
            ptr = &localLabelBuffer[localLabelBufferIndex];
            localLabelBufferIndex += len + 1;
            strcpy(ptr, labelname);

            for(i = 0; i < localLabelCounter; i++) {
                if(strcmp(labelname, localLabelTable[i].name) < 0) break;
            }
            if(i < localLabelCounter) {
                for(; i < localLabelCounter; i++ ) {
                    old_name = localLabelTable[i].name;
                    old_address = localLabelTable[i].address;
                    localLabelTable[i].name = ptr;
                    localLabelTable[i].address = address;
                    ptr = old_name;
                    address = old_address;
                }
            }
            localLabelTable[localLabelCounter].name = ptr;
            localLabelTable[localLabelCounter].address = address;
            localLabelCounter++;
            return true;
        }
        else error(message[ERROR_MAXLOCALLABELS]);
    }
    else error(message[ERROR_LABELDEFINED]);
    return false;
}

bool insertGlobalLabel(char *labelname, int24_t address){
    int index,i,try,len;
    label *tmp;

    //if(findGlobalLabel(labelname)) {
    //    error(message[ERROR_LABELDEFINED]);
    //    return false;
    //}
    len = strlen(labelname);

    // allocate space in buffer for label struct
    tmp = (label *)agon_malloc(sizeof(label));
    if(tmp == 0) return false;

    // allocate space in buffer for string and store it to buffer
    tmp->name = agon_malloc(len+1);
    if(tmp->name == 0) return false;

    strcpy(tmp->name, labelname);
    tmp->address = address;
    index = hash(labelname, GLOBAL_LABEL_TABLE_SIZE);
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++) {
        try = (i + index) % GLOBAL_LABEL_TABLE_SIZE;
        if(globalLabelTable[try] == NULL){
            globalLabelTable[try] = tmp;
            globalLabelCounter++;
            return true;
        }
        if(strcasecmp(globalLabelTable[try]->name, tmp->name) == 0) {
            error(message[ERROR_LABELDEFINED]);
            return false;
        } 
    }
    return false;
}

label * findGlobalLabel(char *name){
    int index,i,try;

    index = hash(name, GLOBAL_LABEL_TABLE_SIZE);
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++){
        try = (index + i) % GLOBAL_LABEL_TABLE_SIZE;
        if(globalLabelTable[try] == NULL){
            return NULL;
        }
        if(globalLabelTable[try] != NULL &&
            strcmp(globalLabelTable[try]->name,name) == 0){
            return globalLabelTable[try];
        }
    }
    return NULL;
}

label *findLabel(char *name) {
    if(name[0] == '@') {
        
        if(((tolower(name[1]) == 'f') || (tolower(name[1]) == 'n')) && name[2] == 0) {
            //printf("NEXT: PD: %d ND: %d Scope: %d\n",an_prev.defined,an_next.defined,filestackCount());
            if(an_next.defined && an_next.scope == filestackCount()) {
                an_return.address = an_next.address;
                return &an_return;
            }
            else return NULL;
        }
        if(((tolower(name[1]) == 'b') || (tolower(name[1]) == 'p')) && name[2] == 0) {
            //printf("PREV: PD: %d ND: %d Scope: %d\n",an_prev.defined,an_next.defined,filestackCount());
            if(an_prev.defined && an_prev.scope == filestackCount()) {
                an_return.address = an_prev.address;
                return &an_return;
            }
            else return NULL;
        }
        
        return findLocalLabel(name);
    }
    else return findGlobalLabel(name);
}