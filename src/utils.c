#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "utils.h"
#include "str2num.h"
#include "label.h"
#include "instruction.h"
#include "./stdint.h"
#include "mos-interface.h"
#include "io.h"

// return a base filename, stripping the given extension from it
void remove_ext (char* myStr, char extSep, char pathSep) {
    char *lastExt, *lastPath;
    // Error checks.
    if (myStr == NULL) return;
    // Find the relevant characters.
    lastExt = strrchr (myStr, extSep);
    lastPath = (pathSep == 0) ? NULL : strrchr (myStr, pathSep);
    // If it has an extension separator.
    if (lastExt != NULL) {
        // and it's to the right of the path separator.
        if (lastPath != NULL) {
            if (lastPath < lastExt) {
                // then remove it.
                *lastExt = '\0';
            }
        } else {
            // Has extension separator with no path separator.
            *lastExt = '\0';
        }
    }
}


bool isEmpty(const char *str){
    return (str[0] == '\0');
}

bool notEmpty(const char *str) {
    return (str[0] != '\0');
}

void error(char* msg)
{
    printf("\"%s\" - line %d - %s\n\r", filename[FILE_CURRENT], linenumber, msg);
    global_errors++;
}

void trimRight(char *str) {
    while(*str) str++;
    str--;
    while(isspace(*str)) str--;
    str++;
    *str = 0;
}

typedef enum {
    TOKEN_REGULAR,
    TOKEN_STRING,
    TOKEN_LITERAL,
    TOKEN_BRACKET
} tokenclass;

// split a 'command.suffix' token in two parts 
void split_suffix(char *mnemonic, char *suffix, char *buffer) {
    bool cmd = true;

    while(*buffer) {
        if(cmd) {
            *mnemonic = *buffer;
            if(*buffer == '.') cmd = false;
            else mnemonic++;
        }
        else *suffix++ = *buffer;
        buffer++;
    }
    *suffix = 0;
    *mnemonic = 0;
}

uint8_t getLineToken(tokentype *token, char *src, char terminator) {
    char *target;
    uint8_t index = 0;
    bool escaped = false;
    bool terminated;
    tokenclass state;

    // remove leading space
    while(*src) {
        if(isspace(*src) != 0) src++;
        else break;
    }
    if(*src == 0) { // empty string
        token->terminator = 0;
        token->start[0] = 0;
        token->length = 0;
        token->next = NULL;
        return 0;
    }
    // copy over the token itself, taking care of the character state within the token
    state = TOKEN_REGULAR;
    target = token->start;
    while(true) {
        terminated = false;
        switch(state) {
            case TOKEN_STRING:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\"':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_LITERAL:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\'':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_BRACKET:
                if(*src == ')') state = TOKEN_REGULAR;
                break;
            case TOKEN_REGULAR:
                if(*src == '\"') state = TOKEN_STRING;
                if(*src == '\'') state = TOKEN_LITERAL;
                if(*src == '(') state = TOKEN_BRACKET;
                terminated = ((*src == ';') || (*src == terminator));
                if(terminator == ' ') terminated = terminated || (*src == '\t'); 
                break;            
        }
        terminated = terminated || (*src == 0);
        if(terminated) {
            token->terminator = *src;
            break;
        }
        *target++ = *src++;
        index++;
    }
    // remove trailing space
    while(index) {
        target--;
        if(isspace(*target) == 0) {
            target++;
            break;
        }
        index--;
    }
    *target = 0;
    if(*src == 0) token->next = NULL;
    else token->next = src+1;
    token->length = index;
    return index;
}

uint8_t getOperatorToken(tokentype *token, char *src) {
    char *target;
    uint8_t index = 0;
    bool literal;

    // remove leading space
    while(*src) {
        if(isspace(*src) != 0) src++;
        else break;
    }
    if(*src == 0) { // empty string
        token->terminator = 0;
        token->start[0] = 0;
        token->length = 0;
        token->next = NULL;
        return 0;
    }

    literal = (*src == '\''); // are we a '' literal value?

    // copy content
    target = token->start;
    while(true) {

        if(literal) {
            if(*src == 0) {
                token->terminator = *src;
                break;
            }
        }
        else {
            if((*src == 0) || (*src == '+') || (*src == '-') || (*src == '*') || (*src == '<') || (*src == '>')) {
                if(((*src == '<') || (*src == '>'))) {
                    if((*(src+1) == *src)) src += 1;
                    else {
                        token->terminator = '!'; // ERROR
                        break;
                    }
                }
                token->terminator = *src;
                break;
            }
        }
        *target++ = *src++;
        index++;
        if(literal && *src == '\'') literal = false;
    }
    // remove trailing space
    while(index) {
        target--;
        if(isspace(*target) == 0) {
            target++;
            break;
        }
        index--;
    }
    *target = 0;
    if(*src == 0) token->next = NULL;
    else token->next = src+1;
    token->length = index;
    return index;
}

#ifdef AGON
int strcasecmp (char *s1, char *s2) {
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  int result;
  if (p1 == p2)
    return 0;
  while ((result = tolower(*p1) - tolower(*p2++)) == 0)
    if (*p1++ == '\0')
      break;
  return result;
}
#endif