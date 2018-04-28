/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a State transaction */
static const char TX_NM[] = "State Tx";

/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet";

/** */
static const bagl_icon_details_t BASE_32_GLYPHS = {
	C_base32_00,	C_base32_01,	C_base32_02,	C_base32_03,
	C_base32_04,	C_base32_05,	C_base32_06,	C_base32_07,
	C_base32_08,	C_base32_09,	C_base32_10,	C_base32_11,
	C_base32_12,	C_base32_13,	C_base32_14,	C_base32_15,
	C_base32_16,	C_base32_17,	C_base32_18,	C_base32_19,
	C_base32_20,	C_base32_21,	C_base32_22,	C_base32_23,
	C_base32_24,	C_base32_25,	C_base32_26,	C_base32_27,
	C_base32_28,	C_base32_29,	C_base32_30,	C_base32_31
};

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

void display_no_public_key() {
	os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[0], NO_PUBLIC_KEY_0, sizeof(NO_PUBLIC_KEY_0));
	os_memmove(current_public_key[1], NO_PUBLIC_KEY_1, sizeof(NO_PUBLIC_KEY_1));

	C_icon_idle = C_icon_default;

	publicKeyNeedsRefresh = 0;
}

void display_public_key(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon) {
	os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));

	C_icon_idle = C_icon;
}

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
