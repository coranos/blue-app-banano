/*
 * MIT License, see root folder for full license.
 */
#ifndef DEBUG_H
#define DEBUG_H
#include <stdbool.h>
#include "os.h"

#define DEBUG_OUT_ENABLED false

#define DEBUG_OUT_LENGTH 0x0100

extern char debug_out[DEBUG_OUT_LENGTH];
extern unsigned int debug_ix;

void clearDebug();

void appendDebug(const bool enable_debug,const void *in, const unsigned int in_length);

void appendDebugInteger(const bool enable_debug, const char prefix, const unsigned int nbr, const char suffix);

void appendDebugUnsignedCharArray(const bool enable_debug, const char prefix, const unsigned char * buffer, const unsigned int length, const char suffix);

#endif // DEBUG_H
