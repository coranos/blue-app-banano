/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"
#include "base-encoding.h"

/** max digits of address */
#define MAX_TX_ADDRESS_DIGITS 64

/** max digits of public key */
#define MAX_PUBLIC_KEY_DIGITS 64


/** current public key text. */
char current_public_key_text[MAX_PUBLIC_KEY_DIGITS];

/** current transaction address text. */
char current_tx_address_text[MAX_TX_ADDRESS_DIGITS];

/** */
static const char NO_KEY[] = "the ledger hasn'tseen a public keyrequest yet from a desktop app";
                            //ban_100000000000|0000000000000000|0000000000000000|0000000000000

/** */
static const char KEY_PREFIX_BAN[] = "BAN_";

static void update_public_key_display() {
	os_memset(current_public_key_display,'\0',sizeof(current_public_key_display));
	unsigned int c = 0;
	for(unsigned int y = 0; y < NUM_TEXT_DISPLAY_LINES; y++) {
		for(unsigned int x = 0; x < NUM_TEXT_DISPLAY_WIDTH-1; x++) {
			current_public_key_display[y][x] = current_public_key_text[c];
			c++;
		}
		current_public_key_display[y][NUM_TEXT_DISPLAY_WIDTH-1] = '\0';
	}
}

void update_no_public_key_data() {
	os_memset(current_public_key_text,'\0',sizeof(current_public_key_text));
	for(unsigned int c = 0; c < sizeof(NO_KEY); c++) {
		current_public_key_text[c] = NO_KEY[c];
	}

	update_public_key_display();

	C_icon_idle = C_icon_default;

	viewNeedsRefresh = 0;
}

void update_public_key_data(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon) {
	os_memset(current_public_key_text,'\0',sizeof(current_public_key_text));
	unsigned int c = 0;
	for(; c < sizeof(KEY_PREFIX_BAN)-1; c++) {
		current_public_key_text[c] = KEY_PREFIX_BAN[c];
	}

	current_public_key_text[c] = '?';
	c++;

	encode_base_32((void *)public_key,sizeof(ed25519_public_key),current_public_key_text+c,sizeof(current_public_key_text)-c);

	update_public_key_display();

	C_icon_idle = C_icon;
}

/** display the screen with the tx address */
void update_tx_address_data(void) {
	for(unsigned int c = 0; c < sizeof(current_tx_address_text); c++) {
		current_tx_address_text[c] = '0';
	}
	for(unsigned int c = 0; c < sizeof(KEY_PREFIX_BAN); c++) {
		current_tx_address_text[c] = KEY_PREFIX_BAN[c];
	}

	current_tx_address_text[sizeof(KEY_PREFIX_BAN)] = '?';

	const unsigned int prefix_offset = sizeof(KEY_PREFIX_BAN) + 1;
	encode_base_32((void *)raw_tx,32,current_tx_address_text+prefix_offset,sizeof(current_tx_address_text)-prefix_offset);

	unsigned int c = 0;
	for(unsigned int y = 0; y < NUM_TEXT_DISPLAY_LINES; y++) {
		for(unsigned int x = 0; x < NUM_TEXT_DISPLAY_WIDTH-1; x++) {
			current_public_key_display[x][y] = current_public_key_text[c];
			c++;
		}
		current_tx_address_display[y][NUM_TEXT_DISPLAY_WIDTH-1] = '\0';
	}
}

/** display the screen with the tx amount */
void update_tx_amount_data(void) {
	for(unsigned int c = 0; c < sizeof(current_tx_amount_text); c++) {
		current_tx_amount_text[c] = ' ';
	}
	encode_base_10((void *)raw_tx,32,current_tx_amount_text,sizeof(current_tx_amount_text));
}
