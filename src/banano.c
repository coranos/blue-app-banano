/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a State transaction */
static const char TX_NM[] = "State Tx";

/** Label when a public key has not been set yet */
static const bagl_icon_details_t * NO_KEY_0[] = {
	//	&C_base32_N,&C_base32_O,NULL
	&C_base32_1,&C_base32_3,NULL
};
/*
   static const bagl_icon_details_t * NO_KEY_1[] = {
   //	&C_base32_K,&C_base32_E,&C_base32_Y
   &C_base32_1,&C_base32_1,&C_base32_1
   };
   static const bagl_icon_details_t * NO_KEY_2[] = {
   //	&C_base32_Y,&C_base32_E,&C_base32_T
   &C_base32_1,&C_base32_1,&C_base32_1
   };
 */

/** array of the glyphs used in base32 conversion */

static const bagl_icon_details_t * BASE_32_GLYPHS = {
	&C_base32_1, &C_base32_3, &C_base32_4, &C_base32_5,
	&C_base32_6, &C_base32_7, &C_base32_8, &C_base32_9,
	&C_base32_A, &C_base32_B, &C_base32_C, &C_base32_D,
	&C_base32_E, &C_base32_F, &C_base32_G, &C_base32_H,
	&C_base32_I, &C_base32_J, &C_base32_K, &C_base32_M,
	&C_base32_N, &C_base32_O, &C_base32_P, &C_base32_Q,
	&C_base32_R, &C_base32_S, &C_base32_T, &C_base32_U,
	&C_base32_W, &C_base32_X, &C_base32_Y, &C_base32_Z
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
	int x;
	int y;
	for(x = 0; x < MAX_TX_ICON_WIDTH; x++) {
		for(y = 0; y < MAX_TX_ICON_LINES; y++) {
			current_public_key[y][x] = C_base32__;
		}
	}

	x = 0;
	if(NO_KEY_0[x] != NULL) {
		current_public_key[0][x] = *NO_KEY_0[x];
	}
	x++;
	if(NO_KEY_0[x] != NULL) {
		current_public_key[0][x] = *NO_KEY_0[x];
	}
	x++;
	if(NO_KEY_0[x] != NULL) {
		current_public_key[0][x] = *NO_KEY_0[x];
	}
//	for(x = 0; x < 2; x++) {
//		if(NO_KEY_0[x] != NULL) {
//			current_public_key[0][x] = *NO_KEY_0[x];
//		}
//	}

	//os_memmove(current_public_key[0], NO_KEY_0, sizeof(NO_KEY_0));
	//os_memmove(current_public_key[1], NO_KEY_1, sizeof(NO_KEY_1));
	//os_memmove(current_public_key[2], NO_KEY_2, sizeof(NO_KEY_2));

	C_icon_idle = C_icon_default;

	publicKeyNeedsRefresh = 0;
}

void display_public_key(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon) {
	//os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	//os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	//os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));

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
