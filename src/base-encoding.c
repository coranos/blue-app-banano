/*
 * MIT License, see root folder for full license.
 */
#include "base-encoding.h"

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };


/** array of base10 alphabet letters */
static const char BASE_32_ALPHABET[] = {
	'1', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'W', 'X', 'Y', 'Z'
};

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len,
                                  const void * in, const unsigned int in_length,
                                  char * out,  const unsigned int out_length,
                                  const bool enable_debug);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_10(const void *in, const unsigned int in_length,
                            char *out, const unsigned int out_length,
                            const bool enable_debug) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length,
	                     enable_debug);
}

/** encodes in_length bytes from in into base-32, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_32(const void *in, const unsigned int in_length,
                            char *out, const unsigned int out_length,
                            const bool enable_debug) {
	return encode_base_x(BASE_32_ALPHABET, sizeof(BASE_32_ALPHABET), in, in_length, out, out_length,
	                     enable_debug);
}

static unsigned int divide_and_remainder(const unsigned char * divided, const unsigned int divided_len,
                                         unsigned char * dividend, const unsigned int dividend_len,
                                         const unsigned int divisor, const unsigned int radix,
                                         const bool enable_debug) {
	unsigned int divided_part = 0;
	unsigned int divided_index = 0;
//	const unsigned int max_divisions = divided_len * radix;
	const unsigned int max_divisions = 0;
	unsigned int cur_divisions = 0;
	while(divided_index < divided_len) {
		cur_divisions++;
		if(cur_divisions > max_divisions) {
			THROW(0x6D16);
		}
		if(divided_part < divisor) {
			divided_part *= radix;
			divided_part += *(divided + divided_index);
			divided_index++;
		} else {
			unsigned int dividend_part = divided_part / divisor;
			for(unsigned int c = 1; c < dividend_len; c++) {
				*(dividend + (c-1)) = *(dividend + c);
			}
			*(dividend + (dividend_len-1)) = dividend_part;
			divided_part -= dividend_part;
		}
	}
	return divided_part;
}

#define BASEX_DIVISION_BUFFER_SIZE 128

#define BASEX_DIVISION_RADIX 256

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes.
 * algorithm:
 * 1) start with a input in base256 (in), and a divisor (alphabet_len).
 * 2) Do long division, dividing the input by the divisor, saving the dividend_length, dividend and remainder.
 * 3) look up the remainder in the alphabet, save it in the output.
 * 4) if dividend_length = 0 return.
 * 4) use the dividend as a new input
 * 5) run steps 2,3,4 until the input
 */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len,
                                  const void * in, const unsigned int in_len_raw,
                                  char * out, const unsigned int out_len,
                                  const bool enable_debug) {
	unsigned char divided[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(divided,0x00,sizeof(divided));

	unsigned char dividend[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(dividend,0x00,sizeof(dividend));

	unsigned char remainders[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(remainders,0x00,sizeof(remainders));

	if(out_len > BASEX_DIVISION_BUFFER_SIZE) {
		unsigned char debug_out_len[4];
		debug_out_len[0] = out_len >> 24;
		debug_out_len[1] = out_len >> 16;
		debug_out_len[2] = out_len >> 8;
		debug_out_len[3] = out_len;
		appendDebug(enable_debug,"\x0A",1);
		appendDebug(enable_debug,debug_out_len,sizeof(debug_out_len));
		appendDebug(enable_debug,"\xA0",3);
		THROW(0x6D11);
	}

	THROW(0x6D21);

	const unsigned int remainders_offset = sizeof(remainders)-out_len;
	const unsigned char * remainders_out = remainders + remainders_offset;

	if(in_len_raw > BASEX_DIVISION_BUFFER_SIZE) {
		THROW(0x6D12);
	}
	os_memmove(divided, in, in_len_raw);
	unsigned int in_len = in_len_raw;
	while ((in_len > 0) && (*divided == 0)) {
		for(unsigned int c = 1; c < in_len; c++) {
			*(divided + (c-1)) = *(divided + c);
		}
		in_len--;
	}

	const unsigned int divided_len = in_len;
	const unsigned int dividend_len = in_len;
	const unsigned int divisor = alphabet_len;
	const unsigned int radix = BASEX_DIVISION_RADIX;

	const unsigned int max_divisions = in_len * radix;
	unsigned int cur_divisions = 0;
	bool empty_divided = false;
	while(!empty_divided) {
		//debug(enable_debug,debug_out,debug_ix_ptr,debug_length,"ED",2);
		cur_divisions++;
		if(cur_divisions > max_divisions) {
			THROW(0x6D15);
		}
		const unsigned int remainder =
		  divide_and_remainder( divided,  divided_len,
		                        dividend, dividend_len,
		                        divisor, radix,
		                        enable_debug);
		for(unsigned int c = 1; c < in_len; c++) {
			*(remainders + (c-1)) = *(remainders + c);
		}
		*(remainders + (in_len-1)) = remainder;

		os_memmove(divided, dividend, in_len);

		bool divided_all_zero = true;
		for(unsigned int c = 0; c < in_len; c++) {
			if(*(divided+c) != 0) {
				divided_all_zero = false;
			}
		}
		if(divided_all_zero) {
			empty_divided = true;
		}
	}
	THROW(0x6D14);

	for(unsigned int c = 0; c < out_len; c++) {
		unsigned char remainder_out = *(remainders_out + c);
		*(out+c) = *(alphabet + remainder_out);
	}
	return out_len;

	// THROW(0x6D21);
	// THROW(0x6D22);
}
