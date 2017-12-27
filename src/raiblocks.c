/*
 * MIT License, see root folder for full license.
 */
#include "raiblocks.h"

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** Label when displaying a Issue transaction */
static const char TX_NM[] = "RaiBlocks Tx";

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc() {

	max_scr_ix = 1;
	curr_scr_ix = 0;

	os_memmove(tx_desc[0][0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(tx_desc[0][1], TX_NM, sizeof(TX_NM));
	os_memmove(tx_desc[0][2], TXT_BLANK, sizeof(TXT_BLANK));

	return 1;
}
