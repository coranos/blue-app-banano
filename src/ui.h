/*
 * MIT License, see root folder for full license.
 */
#ifndef UI_H
#define UI_H

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "bagl.h"
#include "ed25519.h"

/** the idle screen icon */
extern bagl_icon_details_t C_icon_idle;

/** the timer */
extern int exit_timer;

/** length of the APDU (application protocol data unit) header. */
#define APDU_HEADER_LENGTH 5

/** offset in the APDU header which says the length of the body. */
#define APDU_BODY_LENGTH_OFFSET 4

/** for signing, indicates this is the last part of the transaction. */
#define P1_LAST 0x80

/** for signing, indicates this is not the last part of the transaction, there are more parts coming. */
#define P1_MORE 0x00

/** length of BIP44 path */
#define BIP44_PATH_LEN 5

/** length of BIP44 path, in bytes */
#define  BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int))

/**
 * Nano S has 320 KB flash, 10 KB RAM, uses a ST31H320 chip.
 * This effectively limits the max size
 */
#define MAX_TX_RAW_LENGTH 217

/** max width of a single line of iconified text. */
#define MAX_TX_ICON_WIDTH 21

/** max lines of iconified text to display. */
#define MAX_TX_ICON_LINES 3

/** max digits of amount */
#define MAX_TX_AMOUNT_DIGITS 40

/** UI currently displayed */
enum UI_STATE {
	UI_INIT, UI_IDLE, UI_SIGN, UI_TX_ADDR, UX_TX_AMT, UI_DENY, UI_PUBLIC_KEY
};

/** UI state enum */
extern enum UI_STATE uiState;

/** UI state flag */
extern ux_state_t ux;

/** notification to refresh the view, if we are displaying data on the current view */
extern unsigned char viewNeedsRefresh;

/** raw transaction data. */
extern unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
extern unsigned int raw_tx_ix;

/** current length of raw transaction. */
extern unsigned int raw_tx_len;

/** ed25519 signature */
extern ed25519_signature sig;

/** currently displayed public key */
extern bagl_icon_details_t current_public_key[MAX_TX_ICON_LINES][MAX_TX_ICON_WIDTH];

/** currently displayed transaction address */
extern bagl_icon_details_t current_tx_address[MAX_TX_ICON_LINES][MAX_TX_ICON_WIDTH];

/** process a partial transaction */
const bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e);

/** show the idle UI */
void ui_idle(void);

/** show the "Sign TX" ui, starting at the top of the Tx display */
void ui_sign(void);

/** return the length of the communication buffer */
unsigned int get_apdu_buffer_length();

#endif // UI_H
