/*
 * MIT License, see root folder for full license.
 */
#ifndef BASE_ENCODING_H
#define BASE_ENCODING_H

#include <stdbool.h>
#include "debug.h"
#include "os.h"

#define BASEX_DIVISION_BUFFER_SIZE 128

#define BASEX_DIVISION_RADIX 256

/** encodes in_length bytes from in into base-2, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_2(const void *in, const unsigned int in_length,
                            char *out, const unsigned int out_length,
                            const bool enable_debug);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_10(const void *in, const unsigned int in_length,
                            char *out, const unsigned int out_length,
                            const bool enable_debug);

/** encodes in_length bytes from in into base-32, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_32(const void *in, const unsigned int in_length,
                            char *out, const unsigned int out_length,
                            const bool enable_debug);

#endif // BANANO_H
