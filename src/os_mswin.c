#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "colors.h"
#include "typedefs.h"
#include "common.h"

// if typedef doesn't exist (msvc, blah)
typedef intptr_t ssize_t;

int typecolor;
int sexec;
int mmMode = 0;
int markall = 0;

char hlinkstr[6], sizestr[32], majorstr[6], minorstr[6];

char timestyle[9] = "locale";

int writePermsEntry(char * perms, mode_t mode){
  typecolor = DISPLAY_PAIR;

  sexec = 0;

  // Temporary Dummy function
  return typecolor;
}

// To Do, port to Windows
void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count);

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

int setenv(const char *name, const char *value, int overwrite){

  char* envValue;
  int envValueLen = 0;
  int returnVal = -1;

  envValueLen = snprintf(NULL, 0, "%s=%s", name, value);
  envValue = malloc(sizeof(char) * (envValueLen + 1));

  sprintf(envValue, "%s=%s", name, value);

  returnVal =_putenv(envValue);

  return returnVal;
}
