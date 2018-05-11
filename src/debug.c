/*
 * MIT License, see root folder for full license.
 */
#include "debug.h"

char debug_out[DEBUG_OUT_LENGTH];
unsigned int debug_ix = 0;

void clearDebug() {
	os_memset(debug_out,0x00,sizeof(debug_out));
	debug_ix = 0;
}

void appendDebug(const bool enable_debug,const void *in, const unsigned int in_length) {
	if(!enable_debug) {
		return;
	}
	if(debug_ix + in_length >= sizeof(debug_out)) {
		THROW(0x6D19);
	}
	os_memmove(debug_out + debug_ix, in, in_length);
	debug_ix += in_length;
}


void appendDebugInteger(const bool enable_debug, const char prefix, const unsigned int nbr, const char suffix) {
	if(enable_debug) {
		unsigned char buffer[4];
		buffer[0] = nbr >> 24;
		buffer[1] = nbr >> 16;
		buffer[2] = nbr >> 8;
		buffer[3] = nbr;
		appendDebug(enable_debug,&prefix,1);
		appendDebug(enable_debug,buffer,sizeof(buffer));
		appendDebug(enable_debug,&suffix,1);
	}
}

// static void appendDebugUnsignedChar(const bool enable_debug, const char prefix, const unsigned char value, const char suffix) {
//  if(enable_debug) {
//    appendDebug(enable_debug,&prefix,1);
//    appendDebug(enable_debug,&value,1);
//    appendDebug(enable_debug,&suffix,1);
//  }
// }

void appendDebugUnsignedCharArray(const bool enable_debug, const char prefix, const unsigned char * buffer, const unsigned int length, const char suffix) {
	appendDebug(enable_debug,&prefix,1);
	appendDebugInteger(enable_debug,prefix,length,suffix);
	appendDebug(enable_debug,buffer,length);
	appendDebug(enable_debug,&suffix,1);
}
