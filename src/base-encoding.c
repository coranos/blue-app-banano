/*
 * MIT License, see root folder for full license.
 */
#include "base-encoding.h"
#include "os.h"

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };


/** array of base10 alphabet letters */
static const char BASE_32_ALPHABET[] = {
	'1', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'W', 'X', 'Y', 'Z'
};

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
                                  const unsigned int out_length);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_10(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into base-32, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_32(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_32_ALPHABET, sizeof(BASE_32_ALPHABET), in, in_length, out, out_length);
}

#define TMP_SIZE 64

#define BUFFER_SIZE 128

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
                                  const unsigned int out_length) {
	char tmp[TMP_SIZE];
	char buffer[BUFFER_SIZE];
	int inLength = in_length;
	int outLength = out_length;
	int alphabetLen = alphabet_len;
	int bufferIx;
	int startAt;
	int zeroCount = 0;

	if (inLength > TMP_SIZE) {
		THROW(0x6D11);
	}
	os_memmove(tmp, in, inLength);
	while ((zeroCount < inLength) && (tmp[zeroCount] == 0)) {
		++zeroCount;
	}
	// if (alphabet_len == 10) {
		// THROW(0x6D20 + zeroCount);
	// }
	bufferIx = 2 * inLength;
	if (bufferIx > BUFFER_SIZE) {
		THROW(0x6D12);
	}

	startAt = zeroCount;
	while (startAt < inLength) {
		short remainder = 0;
		int divLoop;
		for (divLoop = startAt; divLoop < inLength; divLoop++) {
			unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
			unsigned short tmpDiv = remainder * 256 + digit256;
			tmp[divLoop] = (unsigned char) (tmpDiv / alphabetLen);
			remainder = (tmpDiv % alphabetLen);
		}
		if (tmp[startAt] == 0) {
			++startAt;
		}
		if (remainder >= alphabetLen) {
			THROW(0x6D21);
		}
		if (bufferIx < 0) {
			THROW(0x6D22);
		}
		buffer[--bufferIx] = *(alphabet + remainder);
	}

	while ((bufferIx < (2 * inLength)) && (buffer[bufferIx] == *(alphabet + 0))) {
		++bufferIx;
	}
	while (zeroCount-- > 0) {
		if (bufferIx < 0) {
			THROW(0x6D19);
		}
		buffer[--bufferIx] = *(alphabet + 0);
	}
	if (bufferIx < 0) {
		THROW(0x6D14);
	}
	const int trueOutLength = (2 * inLength) - bufferIx;
	if (trueOutLength < 0) {
		THROW(0x6D15);
	}
	if (trueOutLength > outLength) {
		THROW(0x6D16);
	}
	if(bufferIx >= BUFFER_SIZE) {
		THROW(0x6D17);
	}
	os_memmove(out, (buffer + bufferIx), trueOutLength);
	const unsigned int true_out_length = trueOutLength;
	return true_out_length;
}
