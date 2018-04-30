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

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc(void);

/** displays the "no public key" message, prior to a public key being requested. */
void display_no_public_key(void);

/** displays the public key, assumes length is 64. */
void display_public_key(const ed25519_public_key * public_key, bagl_icon_details_t const C_icon);

#endif // BANANO_H
