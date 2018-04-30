/*
 * MIT License, see root folder for full license.
 */

#ifndef BANANO_H
#define BANANO_H

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"
#include "glyphs.h"

/** update the data on the screen with the tx address */
void update_tx_address_data(void);

/** update the data on the screen with the tx amount */
void update_tx_amount_data(void);

/** displays the "no public key" message, prior to a public key being requested. */
void update_no_public_key_data(void);

/** displays the public key, assumes length is 64. */
void update_public_key_data(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon);

#endif // BANANO_H
