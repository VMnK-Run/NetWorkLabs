#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "result.txt"
#define WriteIn(format, ...) \
        do {\
            FILE *file;\
            file = fopen(FILE_NAME, "a+");\
            fprintf(file, format,  ##__VA_ARGS__);\
            fclose(file);\
        } while(0)

#endif