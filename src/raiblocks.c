/*
 * MIT License, see root folder for full license.
 */
#include "raiblocks.h"

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a Issue transaction */
static const char TX_NM[] = "RaiBlocks Tx";

/** array of capital letter hex values */
static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
static unsigned char next_raw_tx() {
	if (raw_tx_ix < raw_tx_len) {
		unsigned char retval = raw_tx[raw_tx_ix];
		raw_tx_ix += 1;
		return retval;
	} else {
		THROW(0x6D05);
		return 0;
	}
}

/** fills the array in arr with the given number of bytes from raw_tx. */
static void next_raw_tx_arr(unsigned char * arr, unsigned int length) {
	for (unsigned int ix = 0; ix < length; ix++) {
		*(arr + ix) = next_raw_tx();
	}
}

/** converts a byte array in src to a hex array in dest, using only dest_len bytes of dest before stopping. */
static void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len) {
	for (unsigned int src_ix = 0, dest_ix = 0; dest_ix < dest_len; src_ix++, dest_ix += 2) {
		unsigned char src_c = *(src + src_ix);
		unsigned char nibble0 = (src_c >> 4) & 0xF;
		unsigned char nibble1 = src_c & 0xF;

		*(dest + dest_ix + 0) = HEX_CAP[nibble0];
		*(dest + dest_ix + 1) = HEX_CAP[nibble1];
	}
}

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc() {
	max_scr_ix = 1;
	curr_scr_ix = 1;

	os_memmove(tx_desc[0][0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(tx_desc[0][1], TX_NM, sizeof(TX_NM));
	os_memmove(tx_desc[0][2], TXT_BLANK, sizeof(TXT_BLANK));

	unsigned char varPartArr[8];
	char varPartArrHex[16];
	for (int varIx = 0; varIx < 3; varIx++) {
		os_memmove(tx_desc[curr_scr_ix][0], TXT_BLANK, sizeof(TXT_BLANK));
		tx_desc[curr_scr_ix][0][0] = BASE_10_ALPHABET[curr_scr_ix];
		for (int varScreenIx = 0; varScreenIx < 2; varScreenIx++) {
			for (int varScreenLineIx = 1; varScreenLineIx < 3; varScreenLineIx++) {
				next_raw_tx_arr(varPartArr, sizeof(varPartArr));
				to_hex(varPartArrHex, varPartArr, sizeof(varPartArrHex));
				os_memmove(tx_desc[curr_scr_ix][varScreenLineIx], varPartArrHex, sizeof(varPartArrHex));
			}
		}
		max_scr_ix++;
		curr_scr_ix++;
	}

	return 1;
}
