/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"
#include "base-encoding.h"

/** */
static const char NO_KEY0[] = "NO KEY REQUESTED";

/** */
static const char KEY_PREFIX_BAN[] = "BAN_";

/** current public key text. */
static char current_public_key_text[MAX_TX_ICON_WIDTH*MAX_TX_ICON_LINES];

/** current transaction address text. */
static char current_tx_address_text[MAX_TX_ICON_WIDTH*MAX_TX_ICON_LINES];

/** current transaction amount text. */
static char current_tx_amount_text[MAX_TX_AMOUNT_DIGITS];

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

void update_no_public_key_data() {
	display_blank();
	display_no_key();

	C_icon_idle = C_icon_default;

	viewNeedsRefresh = 0;
}

void update_public_key_data(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon) {
	for(unsigned int x = 0; x < MAX_TX_ICON_WIDTH; x++) {
		for(unsigned int y = 0; y < MAX_TX_ICON_LINES; y++) {
			current_public_key[y][x] = C_base32__blank;
		}
	}

	for(unsigned int c = 0; c < sizeof(current_public_key_text); c++) {
		current_public_key_text[c] = ' ';
	}
	for(unsigned int c = 0; c < sizeof(KEY_PREFIX_BAN); c++) {
		current_public_key_text[c] = KEY_PREFIX_BAN[c];
	}

	const unsigned int prefix_offset = sizeof(KEY_PREFIX_BAN) + 1;

	encode_base_32((void *)public_key,sizeof(ed25519_public_key),current_public_key_text+prefix_offset,sizeof(current_public_key_text)-prefix_offset);

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


/** display the screen with the tx address */
void update_tx_address_data(void) {
	for(unsigned int c = 0; c < sizeof(current_public_key_text); c++) {
		current_tx_address_text[c] = ' ';
	}
	for(unsigned int c = 0; c < sizeof(KEY_PREFIX_BAN); c++) {
		current_tx_address_text[c] = KEY_PREFIX_BAN[c];
	}

	const unsigned int prefix_offset = sizeof(KEY_PREFIX_BAN) + 1;

	encode_base_32((void *)raw_tx,32,current_tx_address_text+prefix_offset,sizeof(current_tx_address_text)-prefix_offset);
}

/** display the screen with the tx amount */
void update_tx_amount_data(void) {
	for(unsigned int c = 0; c < sizeof(current_tx_amount_text); c++) {
		current_tx_amount_text[c] = ' ';
	}
	encode_base_10((void *)raw_tx,32,current_tx_amount_text,sizeof(current_tx_amount_text));
}
