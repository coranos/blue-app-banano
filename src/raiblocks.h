/*
 * MIT License, see root folder for full license.
 */

#ifndef NEO_H
#define NEO_H

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc(void);

#endif // NEO_H
