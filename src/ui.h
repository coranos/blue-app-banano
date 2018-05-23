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
#include "banano.h"

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

/** max number of lines of text to display */
#define NUM_TEXT_DISPLAY_LINES 5

/** max width of lines of text to display */
#define NUM_TEXT_DISPLAY_WIDTH 14

/**
 * Nano S has 320 KB flash, 10 KB RAM, uses a ST31H320 chip.
 * This effectively limits the max size
 */
#define MAX_TX_RAW_LENGTH 512

/** max digits of public key */
#define MAX_PUBLIC_KEY_DIGITS 64

/** max digits of address */
#define MAX_TX_ADDRESS_DIGITS 64

#define TX_AMOUNT_BANANO_RAW_DIGITS 30
/** number of banano in a raw.  100000000000000000000000000000 */
/**                             123456789012345678901234567890 */

/** number of banoshi in a raw. 1000000000000000000000000000 */
/**                             1234567890123456789012345678 */
#define TX_AMOUNT_BANOSHI_RAW_DIGITS 28

/** state blocks */
/**
 *	parent(32),
 *	link(32) (sourceBlock / recipient),
 *	representative(32),
 *	balance(16)
 */

/** byte offset and length of parent */
#define TX_PARENT_BYTE_OFFSET 0
#define TX_PARENT_BYTE_LENGTH 32

/** byte offset and length of link (sourceBlock / recipient) */
#define TX_LINK_BYTE_OFFSET TX_PARENT_BYTE_OFFSET+TX_PARENT_BYTE_LENGTH
#define TX_LINK_BYTE_LENGTH 32

/** byte offset and length of representative */
#define TX_REP_BYTE_OFFSET TX_LINK_BYTE_OFFSET+TX_LINK_BYTE_LENGTH
#define TX_REP_BYTE_LENGTH 32

/** byte offset and length of amount */
#define TX_AMOUNT_BYTE_OFFSET TX_REP_BYTE_OFFSET+TX_REP_BYTE_LENGTH
#define TX_AMOUNT_BYTE_LENGTH 16

/** max digits of amount */
#define MAX_TX_AMOUNT_DIGITS 40

/** UI currently displayed */
enum UI_STATE {
	UI_INIT, UI_IDLE, UI_SIGN, UI_TX_ADDR_1,UI_TX_ADDR_2,UI_TX_ADDR_3,UI_TX_ADDR_4, UX_TX_AMT, UI_DENY,
	UI_PUBLIC_KEY_1, UI_PUBLIC_KEY_2, UI_PUBLIC_KEY_3, UI_PUBLIC_KEY_4
};

/** UI state enum */
extern enum UI_STATE uiState;

/** UI state flag */
extern ux_state_t ux;

/** notification to refresh the view, if we are displaying data on the current view */
extern unsigned char viewNeedsRefresh;

/** current public key text. */
extern char current_public_key_display[NUM_TEXT_DISPLAY_LINES][NUM_TEXT_DISPLAY_WIDTH];

/** current transaction address display. +1 for zero end.*/
extern char current_tx_address_display[NUM_TEXT_DISPLAY_LINES][NUM_TEXT_DISPLAY_WIDTH];

/** current transaction amount text. +1 for zero end.*/
extern char current_tx_amount_text[MAX_TX_AMOUNT_DIGITS+1];

/** current transaction amount text, in banoshi. +1 for zero end, +1 for decimal place. */
extern char current_tx_amount_text_banoshi[(MAX_TX_AMOUNT_DIGITS+2)-TX_AMOUNT_BANANO_RAW_DIGITS];

/** raw transaction data. */
extern unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
extern unsigned int raw_tx_ix;

/** current length of raw transaction. */
extern unsigned int raw_tx_len;

/** ed25519 signature */
extern ed25519_signature sig;

/** process a partial transaction */
const bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e);

/** show the idle UI */
void ui_idle(void);

/** show the "Sign TX" ui, starting at the top of the Tx display */
void ui_sign(void);

/** return the length of the communication buffer */
unsigned int get_apdu_buffer_length();

#endif // UI_H
