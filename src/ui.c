/*
 * MIT License, see root folder for full license.
 */

#include "ui.h"
#include "glyphs.h"

/** default font */
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER

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

/** raw transaction data. */
unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
unsigned int raw_tx_ix;

/** current length of raw transaction. */
unsigned int raw_tx_len;

/** ed25519 signature */
ed25519_signature sig;

/** currently displayed public key */
bagl_icon_details_t current_public_key[MAX_TX_ICON_LINES][MAX_TX_ICON_WIDTH];

/** currently displayed transaction address */
bagl_icon_details_t current_tx_address[MAX_TX_ICON_LINES][MAX_TX_ICON_WIDTH];

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
static void ui_public_key(void);

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
		ui_public_key();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		io_seproxyhal_touch_exit(NULL);
		break;
	}

	return 0;
}

#define LINE_1_OFFSET 2
#define LINE_2_OFFSET 12
#define LINE_3_OFFSET 22

/** UI struct for the public key screen */
static const bagl_element_t bagl_ui_public_key_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* first line of description of current public key */
	{ { BAGL_ICON, 0x00, 00, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[0][20], 0, 0, 0, NULL, NULL, NULL, },
	/* second line of description of current public key */
	{ { BAGL_ICON, 0x00, 00, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[1][20], 0, 0, 0, NULL, NULL, NULL, },
	/* third line of description of current public key */
	{ { BAGL_ICON, 0x00, 00, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_public_key[2][20], 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_idle();
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
	{ { BAGL_ICON, 0x00, 00, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_1_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[0][20], 0, 0, 0, NULL, NULL, NULL, },
	/* second line of description of current public key */
	{ { BAGL_ICON, 0x00, 00, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_2_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[1][20], 0, 0, 0, NULL, NULL, NULL, },
	/* third line of description of current public key */
	{ { BAGL_ICON, 0x00, 00, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][0], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 06, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][1], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 12, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][2], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 18, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][3], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 24, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][4], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 30, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][5], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 36, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][6], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 42, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][7], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 48, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][8], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 54, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][9], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 60, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][10], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 66, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][11], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 72, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][12], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 78, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][13], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 84, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][14], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 90, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][15], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00, 96, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][16], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,102, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][17], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,108, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][18], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,114, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][19], 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_ICON, 0x00,120, LINE_3_OFFSET, 6, 10, 0, 0, 0, 0x000000, 0xFFFFFF, 0, 0}, (const char *)&current_tx_address[2][20], 0, 0, 0, NULL, NULL, NULL, },

	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 1, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_RECTANGLE, 0x00, 5, 2,  8, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },

	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 115, 1,  8, 1, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	{ { BAGL_RECTANGLE, 0x00, 113, 2, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },

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
	/* top left bar */
	{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* top right bar */
	{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
	/* center text */
	{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Amount", 0, 0, 0, NULL, NULL, NULL, },
	/* left icon is up arrow  */
	{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
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

/** show the public key screen */
void ui_public_key(void) {
	uiState = UI_PUBLIC_KEY;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		// TODO: add screen for the blue.
		THROW(0x6DBL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos, NULL);
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
