/*
 * MIT License, see root folder for full license.
 */

#ifndef RAIBLOCKS_H
#define RAIBLOCKS_H

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc(void);

#endif // RAIBLOCKS_H
