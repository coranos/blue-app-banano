/*
 * MIT License, see root folder for full license.
 */
#include "banano.h"
#include "base-encoding.h"

/** current public key text. */
char current_public_key_text[MAX_PUBLIC_KEY_DIGITS];

/** current transaction address text. */
char current_tx_address_text[MAX_TX_ADDRESS_DIGITS];

/** the input to the current tx amount */
unsigned char current_tx_amount_char[TX_AMOUNT_BYTE_LENGTH+1];

/** */
static const char NO_KEY[] = "THE LEDGER ISWAITING FOR APUBLIC KEY TOBE  REQUESTEDFROM AN APP.";

/** */
static const char KEY_PREFIX_BAN[] = "BAN_";

static int current_tx_address_display_ix = 0;

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

	encode_base_32((void *)public_key,sizeof(ed25519_public_key),current_public_key_text+c,sizeof(current_public_key_text)-c,
	               false);

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
	encode_base_32((void *)raw_tx,32,current_tx_address_text+prefix_offset,sizeof(current_tx_address_text)-prefix_offset,
	               false);

	current_tx_address_display_ix = 0;
	scroll_current_tx_address_display();
}

void scroll_current_tx_address_display(void) {
	unsigned int c = current_tx_address_display_ix;

	os_memset(current_tx_address_display,'\0',sizeof(current_tx_address_display));

	for(unsigned int x = 0; (x < MAX_TX_ADDRESS_DIGITS) && (c < MAX_PUBLIC_KEY_DIGITS); x++) {
		current_tx_address_display[x] = current_public_key_text[c];
		c++;
	}
	current_tx_address_display[MAX_TX_ADDRESS_DIGITS] = '\0';
	current_tx_address_display_ix++;
	if(current_tx_address_display_ix > (MAX_PUBLIC_KEY_DIGITS - 10)) {
		current_tx_address_display_ix = 0;
	}
}

unsigned int min_tx_len(void) {
	const unsigned int raw_tx_len_except_bip44_and_offset = raw_tx_len - (BIP44_BYTE_LENGTH + TX_AMOUNT_BYTE_OFFSET);
	if(raw_tx_len_except_bip44_and_offset < TX_AMOUNT_BYTE_LENGTH) {
		THROW(0x6D23);
	} else {
		return TX_AMOUNT_BYTE_LENGTH;
	}
}

/** display the screen with the tx amount */
void update_tx_amount_data(void) {
	for(unsigned int c = 0; c < sizeof(current_tx_amount_text); c++) {
		current_tx_amount_text[c] = ' ';
	}

	const unsigned char * raw_tx_amount_offset = raw_tx + TX_AMOUNT_BYTE_OFFSET;
	const unsigned int in_length = min_tx_len();
	for(unsigned int c = 0; c < in_length; c++) {
		current_tx_amount_char[c] = *(raw_tx_amount_offset + c + 1);
	}

	const void * in = (void *)current_tx_amount_char;
	encode_base_10(in,in_length,current_tx_amount_text,sizeof(current_tx_amount_text),
	               false);

	os_memset(current_tx_amount_text_banoshi,'\0',sizeof(current_tx_amount_text_banoshi));

	const unsigned int banoshi_in_length = sizeof(current_tx_amount_text_banoshi);

	/** displaying with no decimal */
	const unsigned int banoshi_in_offset = sizeof(current_tx_amount_text) - banoshi_in_length;
	const char * banoshi_in = current_tx_amount_text + banoshi_in_offset;
	os_memmove(current_tx_amount_text_banoshi,banoshi_in,banoshi_in_offset);

	/** calculating where to put the decimal */
	// const unsigned int banoshi_decimal_offset = TX_AMOUNT_BANANO_RAW_DIGITS - TX_AMOUNT_BANOSHI_RAW_DIGITS;

	// const unsigned int banoshi_in_offset_pre_decimal = sizeof(current_tx_amount_text) - banoshi_in_length;
	// const unsigned int banoshi_in_length_pre_decimal = banoshi_in_length - (banoshi_decimal_offset + 1);
	// const unsigned int banoshi_in_length_post_decimal = banoshi_decimal_offset;
	// const unsigned int banoshi_in_offset_post_decimal = banoshi_in_offset_pre_decimal + banoshi_in_length_pre_decimal;

	// // set the amount before the decimal
	// const char * banoshi_in_pre_decimal = current_tx_amount_text + banoshi_in_offset_pre_decimal;
	// void * banoshi_out_pre_decimal = current_tx_amount_text_banoshi;
	// os_memmove(banoshi_out_pre_decimal,banoshi_in_pre_decimal,banoshi_in_length_pre_decimal);
	//
	// // set the decimal
	// char * banoshi_out_decimal = current_tx_amount_text_banoshi + banoshi_in_length_pre_decimal;
	// *banoshi_out_decimal = '.';
	//
	// // set the amount after the decimal
	// const char * banoshi_in_post_decimal = current_tx_amount_text + banoshi_in_offset_post_decimal;
	// char * banoshi_out_post_decimal = current_tx_amount_text_banoshi + banoshi_in_length_pre_decimal + 1;
	// os_memmove(banoshi_out_post_decimal,banoshi_in_post_decimal,banoshi_in_length_post_decimal);

}
