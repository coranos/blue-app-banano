/*
 * MIT License, see root folder for full license.
 */

#include "ui.h"
#include "glyphs.h"

/** default font */
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER

/** description font */
#define DESCRIPTION_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** the idle screen icon */
bagl_icon_details_t C_icon_idle;

/** the timer */
int exit_timer;

/** UI state enum */
enum UI_STATE uiState;

/** UI state flag */
ux_state_t ux;

/** notification to refresh the view, if we are displaying the public key */
unsigned char viewNeedsRefresh;

/** current public key text. */
char current_public_key_display[NUM_TEXT_DISPLAY_LINES][NUM_TEXT_DISPLAY_WIDTH];

/** current transaction address display. +1 for zero end.*/
char current_tx_address_display[MAX_TX_ADDRESS_DIGITS+1];

/** current transaction amount text. +1 for zero end.*/
char current_tx_amount_text[MAX_TX_AMOUNT_DIGITS+1];

/** current transaction amount text, in banoshi. +1 for zero end, +1 for decimal place. */
char current_tx_amount_text_banoshi[(MAX_TX_AMOUNT_DIGITS+2)-TX_AMOUNT_BANANO_RAW_DIGITS];

/** raw transaction data. */
unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
unsigned int raw_tx_ix;

/** current length of raw transaction. */
unsigned int raw_tx_len;

/** ed25519 signature */
ed25519_signature sig;

/** UI was touched indicating the user wants to exit the app */
static const bagl_element_t * io_seproxyhal_touch_exit(const bagl_element_t *e);

/** UI was touched indicating the user wants to deny te signature request */
static const bagl_element_t * io_seproxyhal_touch_deny(const bagl_element_t *e);

/** display the transaction address */
static void ui_display_tx_address(void);

/** display the transaction amount */
static void ui_display_tx_amount(void);

/** display the UI for signing a transaction */
void ui_sign(void);

/** display the UI for denying a transaction */
static void ui_deny(void);

/** show the public key screen */
static void ui_public_key_1(void);

/** show the public key screen */
static void ui_public_key_2(void);

/** show the public key screen */
static void ui_public_key_3(void);

/** show the public key screen */
static void ui_public_key_4(void);

/** move up in the transaction description list */
static const bagl_element_t * tx_desc_up(const bagl_element_t *e);

/** move down in the transaction description list */
static const bagl_element_t * tx_desc_dn(const bagl_element_t *e);

/** UI struct for the idle screen */
static const bagl_element_t bagl_ui_idle_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* bananos icon */
	{ { BAGL_ICON, 0x00, 40, 14, 104, 32, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&C_icon_idle, 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is a X */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_idle_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_1();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		io_seproxyhal_touch_exit(NULL);
		break;
	}

	return 0;
}

#define LINE_1_OFFSET 19
#define LINE_2_OFFSET 30
#define LINE_X_INDENT 10

/** UI struct for the public key screen */
static const bagl_element_t bagl_ui_public_key_nanos_1[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[0], 0, 0, 0, NULL, NULL, NULL, },
/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[1], 0, 0, 0, NULL, NULL, NULL, },
/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* right icon is down arrow */
	{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_1_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_2();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_3();
		break;
	}
	return 0;
}

/** UI struct for the public key screen */
static const bagl_element_t bagl_ui_public_key_nanos_2[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[1], 0, 0, 0, NULL, NULL, NULL, },
/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[2], 0, 0, 0, NULL, NULL, NULL, },
/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* right icon is down arrow */
	{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_2_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_3();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_1();
		break;
	}
	return 0;
}


/** UI struct for the public key screen */
static const bagl_element_t bagl_ui_public_key_nanos_3[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[2], 0, 0, 0, NULL, NULL, NULL, },
	/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[3], 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* right icon is down arrow */
	{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_3_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_4();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_2();
		break;
	}
	return 0;
}


/** UI struct for the public key screen */
static const bagl_element_t bagl_ui_public_key_nanos_4[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[3], 0, 0, 0, NULL, NULL, NULL, },
	/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, current_public_key_display[4], 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* right icon is down arrow */
	{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_4_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_1();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_3();
		break;
	}
	return 0;
}

/** UI struct for the  "Sign Transaction" screen, Nano S. */
static const bagl_element_t bagl_ui_sign_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* center text */
	{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Sign Tx", 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* right icon is down arrow */
	{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the top "Sign Transaction" screen
 *
 * up on Left button, down on right button, sign on both buttons.
 */
static unsigned int bagl_ui_sign_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_approve(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/** UI struct for the bottom "Deny Transaction" screen, Nano S. */
static const bagl_element_t bagl_ui_deny_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* center text */
	{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Deny Tx", 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* right icon is down arrow  */
	{ { BAGL_ICON, 0x00, 117, 13, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the bottom "Deny Transaction" screen
 *
 * up on Left button, down on right button, deny on both buttons.
 */
static unsigned int bagl_ui_deny_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_deny(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/** UI struct for the transaction address screen, Nano S. */
static const bagl_element_t bagl_ui_tx_address_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, "Address", 0, 0, 0, NULL, NULL, NULL, },
	/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 32 /* address text scrolls */}, current_tx_address_display, 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* right icon is down arrow  */
	{ { BAGL_ICON, 0x00, 117, 13, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },

/* */
};

/**
 * buttons for the transaction address screen
 *
 * up on Left button, down on right button.
 */
static unsigned int bagl_ui_tx_address_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}


/** UI struct for the transaction amount screen, Nano S. */
static const bagl_element_t bagl_ui_tx_amount_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* first line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_1_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 0}, "Amount", 0, 0, 0, NULL, NULL, NULL, },
/* second line of description of current public key */
	{ { BAGL_LABELINE, 0x02, LINE_X_INDENT, LINE_2_OFFSET, 108, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DESCRIPTION_FONT, 32 /* amount text scrolls */}, current_tx_amount_text_banoshi, 0, 0, 0, NULL, NULL, NULL, },
/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* right icon is down arrow  */
	{ { BAGL_ICON, 0x00, 117, 13, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the bottom "Deny Transaction" screen
 *
 * up on Left button, down on right button, deny on both buttons.
 */
static unsigned int bagl_ui_tx_amount_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}


/** if the user wants to exit go back to the app dashboard. */
static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
	// Go back to the dashboard
	os_sched_exit(0);
	return NULL; // do not redraw the widget
}

/** processes the Up button */
static const bagl_element_t * tx_desc_up(const bagl_element_t *e) {
	switch (uiState) {
	case UI_SIGN:
		ui_deny();
		break;

	case UI_TX_ADDR:
		ui_sign();
		break;

	case UX_TX_AMT:
		ui_display_tx_address();
		break;

	case UI_DENY:
		ui_display_tx_amount();
		break;

	default:
		THROW(0x6D02);
		break;
	}
	return NULL;
}

/** processes the Down button */
static const bagl_element_t * tx_desc_dn(const bagl_element_t *e) {
	switch (uiState) {
	case UI_SIGN:
		ui_display_tx_address();
		break;

	case UI_TX_ADDR:
		ui_display_tx_amount();
		break;

	case UX_TX_AMT:
		ui_deny();
		break;

	case UI_DENY:
		ui_sign();
		break;

	default:
		THROW(0x6D01);
		break;
	}
	return NULL;
}

/** processes the transaction approval. the UI is only displayed when all of the TX has been sent over for signing. */
const bagl_element_t*io_seproxyhal_touch_approve(const bagl_element_t *e) {
	os_memmove(G_io_apdu_buffer, sig, sizeof(sig));
	unsigned int tx = sizeof(sig);

	raw_tx_ix = 0;
	raw_tx_len = 0;

	G_io_apdu_buffer[tx++] = 0x90;
	G_io_apdu_buffer[tx++] = 0x00;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
	// Display back the original UX
	ui_idle();
	return 0; // do not redraw the widget
}

/** deny signing. */
static const bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
	raw_tx_ix = 0;
	raw_tx_len = 0;
	G_io_apdu_buffer[0] = 0x69;
	G_io_apdu_buffer[1] = 0x85;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
	// Display back the original UX
	ui_idle();
	return 0; // do not redraw the widget
}

/** show the public key screen 1 */
void ui_public_key_1(void) {
	uiState = UI_PUBLIC_KEY_1;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		// TODO: add screen for the blue.
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_1, NULL);
	}
}

/** show the public key screen 2 */
void ui_public_key_2(void) {
	uiState = UI_PUBLIC_KEY_2;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		// TODO: add screen for the blue.
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_2, NULL);
	}
}

/** show the public key screen 3 */
void ui_public_key_3(void) {
	uiState = UI_PUBLIC_KEY_3;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		// TODO: add screen for the blue.
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_3, NULL);
	}
}

/** show the public key screen 4 */
void ui_public_key_4(void) {
	uiState = UI_PUBLIC_KEY_4;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		// TODO: add screen for the blue.
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_4, NULL);
	}
}


/** show the idle screen. */
void ui_idle(void) {
	uiState = UI_IDLE;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_idle_nanos, NULL);
	}
}

/** show the transaction address screen. */
static void ui_display_tx_address(void) {
	uiState = UI_TX_ADDR;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_tx_address_nanos, NULL);
	}
}

/** show the transaction amount screen. */
static void ui_display_tx_amount(void) {
	uiState = UX_TX_AMT;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_tx_amount_nanos, NULL);
	}
}

/** show the "Sign Transaction" screen. */
void ui_sign(void) {
	uiState = UI_SIGN;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_sign_nanos, NULL);
	}
}

/** show the "deny" screen */
static void ui_deny(void) {
	uiState = UI_DENY;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_deny_nanos, NULL);
	}
}

/** returns the length of the transaction in the buffer. */
unsigned int get_apdu_buffer_length() {
	unsigned int len0 = G_io_apdu_buffer[APDU_BODY_LENGTH_OFFSET];
	return len0;
}
