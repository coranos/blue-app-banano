/*
 * MIT License, see root folder for full license.
 */
#include "base-encoding.h"

/** array of base10 aplhabet letters */
static const char BASE_2_ALPHABET[] = { '0', '1' };

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

/** encodes in_length bytes from in into base-2, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_2(const void *in, const unsigned int in_length,
                           char *out, const unsigned int out_length,
                           const bool enable_debug) {
	return encode_base_x(BASE_2_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length,
	                     enable_debug);
}
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

/**
 * this algorithm simulates a single step in long division.
 * for example:
 *  dividing 10 by 10 should get 1 remainder 0.
 *  dividing 11 by 10 should get 1 remainder 1.
 *  dividing 0D (13) by 10 should get 1 remainder 3
 * 6b6b00000001b60db6 => send 0d (13) to divide_and_remainder.
 *   9a00000000a9
 *   aa00000000aa
 *   ba00000001ab
 * cbcb00000010bc00000000000000000000000000000001bc => divide_and_remainder has remainder of 0x01.
 *   da0000000dad
 *   ea00000003ae => after subtraction 3.
 */
static unsigned int divide_and_remainder(const unsigned char * divided, const unsigned int divided_len,
                                         unsigned char * dividend, const unsigned int dividend_len,
                                         const unsigned int divisor, const unsigned int radix,
                                         const bool enable_debug) {
	bool divide_and_remainder_debug = false;
	unsigned int divided_part = 0;
	unsigned int division_index = 0;
	const unsigned int max_divisions = divided_len * radix;
	// const unsigned int max_divisions = 1;
	unsigned int cur_divisions = 0;
	while(division_index < divided_len) {
		cur_divisions++;
		if(cur_divisions > max_divisions) {
			THROW(0x6D16);
		}
		appendDebugInteger(divide_and_remainder_debug,'\x9C',divided_part,'\xC9');
		appendDebugInteger(divide_and_remainder_debug,'\xAC',division_index,'\xCA');

		// if the divided part is less than the divisor, add another digit.
		// I.E: if you are dividing 31 by 4, and the divided part is 3, multiply 3 by 10 and add the 1.
		if(divided_part < divisor) {
			divided_part *= radix;
			divided_part += *(divided + division_index);
			division_index++;
			// shift the dividend left, and set lowest byte to zero.
			for(unsigned int ix = 1; ix < dividend_len; ix++) {
				*(dividend + (ix-1)) = *(dividend + (ix));
			}
			*(dividend + (dividend_len-1)) = 0;
		}
		// if the divided part is now greater than the divisor, divide and take the remainder.
		// this is done in a seperate conditional statement than the previous,
		// because you are modifying the divided_part in the previous conditional.
		// so the two if statements look like they can be combined but they cannot be combined.
		if(divided_part >= divisor) {
			if(division_index > divided_len) {
				THROW(0x6D23);
			}
			unsigned int dividend_part = divided_part / divisor;
			appendDebugInteger(divide_and_remainder_debug,'\xBC',dividend_part,'\xCB');
			appendDebugInteger(divide_and_remainder_debug,'\xCC',division_index,'\xCC');
			appendDebugUnsignedCharArray(divide_and_remainder_debug,'\xCD',dividend,dividend_len,'\xDC');
			*(dividend + (dividend_len-1)) = dividend_part;
			appendDebugUnsignedCharArray(divide_and_remainder_debug,'\xDD',dividend,dividend_len,'\xDD');
			if(dividend_part > divided_part) {
				THROW(0x6D22);
			}
			appendDebugInteger(divide_and_remainder_debug,'\xDC',divided_part,'\xCD');
			divided_part -= (dividend_part * divisor);
		}
		appendDebugInteger(divide_and_remainder_debug,'\xEC',divided_part,'\xCE');
		appendDebugInteger(divide_and_remainder_debug,'\xFC',division_index,'\xCF');
	}
	return divided_part;
}

static bool dividedIsEmpty(const unsigned char * divided, const unsigned int divided_len) {
	bool divided_all_zero = true;
	for(unsigned int c = 0; c < divided_len; c++) {
		if(*(divided+c) != 0) {
			divided_all_zero = false;
		}
	}
	bool empty_divided = false;
	if(divided_all_zero) {
		empty_divided = true;
	}
	return empty_divided;
}

/**
 * set in_len to be the initial length (in_len_raw) but strip off all leading zeros.
 * for each leading zero, subtract 1 from in_len, and shift the value in divided left one.
 */
unsigned int remove_zeros(unsigned char * divided, const unsigned int divided_len_raw) {
	// removes zeros from left side.
	unsigned int divided_len = divided_len_raw;
	while((divided_len > 0) && ((*divided) == 0x00)) {
		for(unsigned int c = 1; c < divided_len; c++) {
			*(divided + (c-1)) = *(divided + c);
		}
		divided_len--;
	}

	// removes zeros from right side.
	// while((divided_len > 0) && ((*divided + (divided_len-1)) == 0x00)) {
	//  for(unsigned int c = 1; c < divided_len; c++) {
	//    *(divided + (c-1)) = *(divided + c);
	//  }
	//  divided_len--;
	// }
	return divided_len;
}

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

	// divided (the thing to be divided)
	// set to all zeros
	unsigned char divided[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(divided,0x00,sizeof(divided));

	// dividend (the primary result of division)
	// set to all zeros
	unsigned char dividend[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(dividend,0x00,sizeof(dividend));

	// remainders (the secondary result of division, whatever remained after whole number division)
	// set to all zeros
	unsigned char remainders[BASEX_DIVISION_BUFFER_SIZE];
	os_memset(remainders,0x00,sizeof(remainders));

	const bool trace_debug = false;

	// if out_len is too big, log the actual length, and throw an error.
	appendDebugInteger(trace_debug,'\x0A',out_len,'\xA0');
	if(out_len > BASEX_DIVISION_BUFFER_SIZE) {
		THROW(0x6D11);
	}

	// remainders_offset, how many remainders there should be, equal to the output size.
	const unsigned int remainders_offset = sizeof(remainders)-out_len;
	// remainders_out, the pointer to the first remainder that will be saved (all before this will be zero).
	unsigned char * remainders_out = remainders + remainders_offset;
	// the current index into the remainders output.
	int remainders_out_ix = out_len-1;

	// if the input length is too big, log it and throw an error.
	appendDebugInteger(trace_debug,'\x1A',in_len_raw,'\xA1');
	if(in_len_raw > BASEX_DIVISION_BUFFER_SIZE) {
		THROW(0x6D12);
	}

	appendDebugUnsignedCharArray(enable_debug,'\x2B',in,in_len_raw,'\xB2');

	// make the first "divided" be the input.
	os_memmove(divided, in, in_len_raw);

	unsigned int working_len = remove_zeros(divided, in_len_raw);

	appendDebugInteger(trace_debug,'\x3A',working_len,'\xA3');

	const unsigned int divisor = alphabet_len;
	const unsigned int radix = BASEX_DIVISION_RADIX;

	appendDebugUnsignedCharArray(trace_debug,'\x4B',divided,working_len,'\xB4');

	const unsigned int max_divisions = working_len * radix;
	// const unsigned int max_divisions = 1;
	unsigned int cur_divisions = 0;
	bool empty_divided = dividedIsEmpty(divided,working_len);
	while(!empty_divided) {
		cur_divisions++;

		appendDebugInteger(trace_debug,'\x5A',cur_divisions,'\xA5');
		appendDebugUnsignedCharArray(enable_debug,'\x6B',divided,working_len,'\xB6');

		if(cur_divisions > max_divisions) {
			THROW(0x6D15);
		}

		const unsigned int remainder =
		  divide_and_remainder( divided,  working_len,
		                        dividend, working_len,
		                        divisor, radix,
		                        enable_debug);

		appendDebugInteger(trace_debug,'\x7A',remainder,'\xA7');

		if(remainder >= divisor) {
			THROW(0x6D21);
		}

		if(remainders_out_ix < 0) {
			THROW(0x6D24);
		}
		*(remainders_out + remainders_out_ix) = remainder;

		remainders_out_ix--;
		appendDebugUnsignedCharArray(enable_debug,'\x8B',
		                             remainders_out + remainders_out_ix,
		                             out_len - remainders_out_ix,'\xB8');

		os_memmove(divided, dividend, working_len);
		appendDebugUnsignedCharArray(enable_debug,'\xFB',divided,working_len,'\xBF');
		working_len = remove_zeros(divided, working_len);
		appendDebugUnsignedCharArray(enable_debug,'\xFC',divided,working_len,'\xCF');
		os_memset(dividend,0x00,sizeof(dividend));

		empty_divided = dividedIsEmpty(divided,working_len);
	}

	for(unsigned int c = 0; c < out_len; c++) {
		unsigned char remainder_out = *(remainders_out + c);
		*(out+c) = *(alphabet + remainder_out);
	}
	return out_len;
}
