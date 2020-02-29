#ifndef LOG_H
#define LOG_H

//Initialize logger: -1 error opening, 0 opened already, 1 success
int initLogger(const char *name);
//Log message
void logWrite(const char *format, ...);
//Close logger
void closeLogger();

#endif //LOG_H