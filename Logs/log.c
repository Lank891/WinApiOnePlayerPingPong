#include <stdio.h>
#include <stdarg.h>
#include "log.h"

//File to store logs
static FILE *logFile = NULL;

int initLogger(const char *name) {
    //If fle is opened just leave with 0
    if(logFile) return 0;
    //Open file
    logFile = fopen(name, "w");
    if(logFile) return 1; //Succesfully opened file
    else return -1; //Error while opening
}

void logWrite(const char *format, ...) {
    //If file is not opened just leave
    if(!logFile) return;

    //Get arguments and print them with vfprintf to file
    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args);
    va_end(args);
}

void closeLogger() {
    if(logFile) fclose(logFile);
    logFile = NULL;
}