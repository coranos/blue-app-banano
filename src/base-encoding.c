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

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_10(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into base-32, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_32(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_32_ALPHABET, sizeof(BASE_32_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length) {
	char tmp[64];
	char buffer[128];
	unsigned char buffer_ix;
	unsigned char startAt;
	unsigned char zeroCount = 0;
	if (in_length > sizeof(tmp)) {
		THROW(0x6D11);
	}
	os_memmove(tmp, in, in_length);
	while ((zeroCount < in_length) && (tmp[zeroCount] == 0)) {
		++zeroCount;
	}
	buffer_ix = 2 * in_length;
	if (buffer_ix > sizeof(buffer)) {
		THROW(0x6D12);
	}

	startAt = zeroCount;
	while (startAt < in_length) {
		unsigned short remainder = 0;
		unsigned char divLoop;
		for (divLoop = startAt; divLoop < in_length; divLoop++) {
			unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
			unsigned short tmpDiv = remainder * 256 + digit256;
			tmp[divLoop] = (unsigned char) (tmpDiv / alphabet_len);
			remainder = (tmpDiv % alphabet_len);
		}
		if (tmp[startAt] == 0) {
			++startAt;
		}
		buffer[--buffer_ix] = *(alphabet + remainder);
	}
	while ((buffer_ix < (2 * in_length)) && (buffer[buffer_ix] == *(alphabet + 0))) {
		++buffer_ix;
	}
	while (zeroCount-- > 0) {
		buffer[--buffer_ix] = *(alphabet + 0);
	}
	const unsigned int true_out_length = (2 * in_length) - buffer_ix;
	if (true_out_length > out_length) {
		THROW(0x6D14);
	}
	os_memmove(out, (buffer + buffer_ix), true_out_length);
	return true_out_length;
}

