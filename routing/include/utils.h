#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>

// 定义输出文件
#define FILE_NAME "result.txt"

// 输出到指定文件中
#define WriteIn(format, ...) \
        do {\
            FILE *file;\
            file = fopen(FILE_NAME, "a+");\
            fprintf(file, format,  ##__VA_ARGS__);\
            fclose(file);\
        } while(0)

#define WriteInNoArgs(format) \
        do {\
            FILE *file;\
            file = fopen(FILE_NAME, "a+");\
            fprintf(file, format);\
            fclose(file);\
        } while(0)

#endif