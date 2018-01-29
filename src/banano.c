/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a Issue transaction */
static const char TX_NM[] = "RaiBlocks Tx";

/** array of capital letter hex values */
//static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** array of base10 aplhabet letters */
//static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
//static unsigned char next_raw_tx() {
//	if (raw_tx_ix < raw_tx_len) {
//		unsigned char retval = raw_tx[raw_tx_ix];
//		raw_tx_ix += 1;
//		return retval;
//	} else {
//		THROW(0x6D05);
//		return 0;
//	}
//}

/** skips the given number of bytes in the raw_tx buffer. If this goes off the end of the buffer, throw an error. */
//static void skip_raw_tx(unsigned int tx_skip) {
//	raw_tx_ix += tx_skip;
//	if (raw_tx_ix >= raw_tx_len) {
//		THROW(0x6D03);
//	}
//}

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc() {
	max_scr_ix = 1;
	curr_scr_ix = 1;

	os_memmove(tx_desc[0][0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(tx_desc[0][1], TX_NM, sizeof(TX_NM));
	os_memmove(tx_desc[0][2], TXT_BLANK, sizeof(TXT_BLANK));

//	skip_raw_tx(32);
//	skip_raw_tx(32);
//	skip_raw_tx(32);

	return 1;
}
