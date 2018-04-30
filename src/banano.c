/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"
#include "base-encoding.h"

/** */
static const char NO_KEY0[] = "NO KEY REQUESTED";

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a State transaction */
static const char TX_NM[] = "State Tx";

/** array of capital letter hex values */
//static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** current public key text. */
static char current_public_key_text[MAX_TX_ICON_WIDTH*MAX_TX_ICON_LINES];

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
void display_blank() {
	for(int x = 0; x < MAX_TX_ICON_WIDTH; x++) {
		for(int y = 0; y < MAX_TX_ICON_LINES; y++) {
			current_public_key[y][x] = C_base32__blank;
		}
	}
}

const bagl_icon_details_t * glyph(const char c) {
	switch (c) {
	case '1':
		return &C_base32_1;
	case '3':
		return &C_base32_3;
	case '4':
		return &C_base32_4;
	case '5':
		return &C_base32_5;
	case '6':
		return &C_base32_6;
	case '7':
		return &C_base32_7;
	case '8':
		return &C_base32_8;
	case '9':
		return &C_base32_9;
	case 'A':
		return &C_base32_A;
	case 'B':
		return &C_base32_B;
	case 'C':
		return &C_base32_C;
	case 'D':
		return &C_base32_D;
	case 'E':
		return &C_base32_E;
	case 'F':
		return &C_base32_F;
	case 'G':
		return &C_base32_G;
	case 'H':
		return &C_base32_H;
	case 'I':
		return &C_base32_I;
	case 'J':
		return &C_base32_J;
	case 'K':
		return &C_base32_K;
	case 'M':
		return &C_base32_M;
	case 'N':
		return &C_base32_N;
	case 'O':
		return &C_base32_O;
	case 'P':
		return &C_base32_P;
	case 'Q':
		return &C_base32_Q;
	case 'R':
		return &C_base32_R;
	case 'S':
		return &C_base32_S;
	case 'T':
		return &C_base32_T;
	case 'U':
		return &C_base32_U;
	case 'W':
		return &C_base32_W;
	case 'X':
		return &C_base32_X;
	case 'Y':
		return &C_base32_Y;
	case 'Z':
		return &C_base32_Z;
	case ' ':
		return &C_base32__blank;
	case '_':
		return &C_base32__underscore;

	default:
		return &C_base32__error;
	}
}

void display_no_key() {
	for(unsigned int x = 0; x < sizeof(NO_KEY0); x++) {
		current_public_key[0][x] = *glyph(NO_KEY0[x]);
	}
}

void display_no_public_key() {
	display_blank();
	display_no_key();

	C_icon_idle = C_icon_default;

	viewNeedsRefresh = 0;
}

void display_public_key(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon) {
	for(unsigned int x = 0; x < MAX_TX_ICON_WIDTH; x++) {
		for(unsigned int y = 0; y < MAX_TX_ICON_LINES; y++) {
			current_public_key[y][x] = C_base32__blank;
		}
	}

	for(unsigned int c = 0; c < sizeof(current_public_key_text); c++) {
		current_public_key_text[c] = ' ';
	}
	current_public_key_text[0] = 'B';
	current_public_key_text[1] = 'A';
	current_public_key_text[2] = 'N';
	current_public_key_text[3] = '_';

	encode_base_32((void *)public_key,sizeof(ed25519_public_key),current_public_key_text+5,sizeof(current_public_key_text)-5);

	display_blank();

	unsigned int c = 0;
	for(unsigned int y = 0; y < MAX_TX_ICON_LINES; y++) {
		for(unsigned int x = 0; x < MAX_TX_ICON_WIDTH; x++) {
			current_public_key[y][x] = *glyph(current_public_key_text[c]);
			c++;
		}
	}

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
