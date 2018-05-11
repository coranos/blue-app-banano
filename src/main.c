/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include "os.h"
#include "cx.h"
#include <stdbool.h>

#include "ed25519.h"
#include "os_io_seproxyhal.h"
#include "ui.h"
#include "bagl.h"
#include "banano.h"
#include "base-encoding.h"

#define MAX_EXIT_TIMER 4098

#define EXIT_TIMER_REFRESH_INTERVAL 512

static void Timer_Tick() {
	if (exit_timer > 0) {
		exit_timer--;
	}
}

static void Timer_Set() {
	exit_timer = MAX_EXIT_TIMER;
}

static void Timer_Restart() {
	if(exit_timer != MAX_EXIT_TIMER) {
		Timer_Set();
	}
}

static bool Timer_Expired() {
	return exit_timer <= 0;
}

/** IO buffer to communicate with the outside world. */
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

/** start of the buffer, reject any transmission that doesn't start with this, as it's invalid. */
#define CLA 0x80

/** #### instructions start #### **/
/** instruction to sign transaction and send back the signature. */
#define INS_SIGN 0x02

/** instruction to send back the public key. */
#define INS_GET_PUBLIC_KEY 0x04

/** instruction to send back the public key, and a signature of the private key signing the public key. */
#define INS_GET_SIGNED_PUBLIC_KEY 0x08

/** instruction to send back the base 10 encoded version of the value. */
#define INS_GET_BASE_10_ENCODED 0x10

/** #### instructions end #### */

/** some kind of event loop */
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
	switch (channel & ~(IO_FLAGS)) {
	case CHANNEL_KEYBOARD:
		break;

	// multiplexed io exchange over a SPI channel and TLV encapsulated protocol
	case CHANNEL_SPI:
		if (tx_len) {
			io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

			if (channel & IO_RESET_AFTER_REPLIED) {
				reset();
			}
			// nothing received from the master so far
			//(it's a tx transaction)
			return 0;
		} else {
			return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
		}

	default:
		THROW(INVALID_PARAMETER);
	}
	return 0;
}


/** refreshes the display if the public key was changed ans we are on the page displaying the public key */
static void refresh_public_key_display(void) {
	switch(uiState) {
	case UI_PUBLIC_KEY_1:
	case UI_PUBLIC_KEY_2:
	case UI_PUBLIC_KEY_3:
	case UI_PUBLIC_KEY_4:
		viewNeedsRefresh = 1;
		break;
	default:
		break;
	}
}

static void refresh_idle_display(void) {
	if (uiState == UI_IDLE) {
		viewNeedsRefresh = 1;
	}
}

/** main loop. */
static void banano_main(void) {
	volatile unsigned int rx = 0;
	volatile unsigned int tx = 0;
	volatile unsigned int flags = 0;

	// DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
	// goal is to retrieve APDU.
	// When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
	// sure the io_event is called with a
	// switch event, before the apdu is replied to the bootloader. This avoid
	// APDU injection faults.
	for (;; ) {
		volatile unsigned short sw = 0;

		BEGIN_TRY
		{
			TRY
			{
				rx = tx;
				// ensure no race in catch_other if io_exchange throws an error
				tx = 0;
				rx = io_exchange(CHANNEL_APDU | flags, rx);
				flags = 0;

				// no apdu received, well, reset the session, and reset the
				// bootloader configuration
				if (rx == 0) {
					THROW(0x6982);
				}

				// if the buffer doesn't start with the magic byte, return an error.
				if (G_io_apdu_buffer[0] != CLA) {
					THROW(0x6E00);
				}

				// check the second byte (0x01) for the instruction.
				switch (G_io_apdu_buffer[1]) {

				// we're getting a transaction to sign, in parts.
				case INS_SIGN: {
					Timer_Restart();
					// check the third byte (0x02) for the instruction subtype.
					if ((G_io_apdu_buffer[2] != P1_MORE) && (G_io_apdu_buffer[2] != P1_LAST)) {
						THROW(0x6A86);
					}

					// move the contents of the buffer into raw_tx, and update raw_tx_ix to the end of the buffer, to be ready for the next part of the tx.
					unsigned int len = get_apdu_buffer_length();
					unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH;
					unsigned char * out = raw_tx + raw_tx_ix;
					if (raw_tx_ix + len > MAX_TX_RAW_LENGTH) {
						THROW(0x6D08);
					}
					os_memmove(out, in, len);
					raw_tx_ix += len;

					// set the buffer to end with a zero.
					G_io_apdu_buffer[APDU_HEADER_LENGTH + len] = '\0';

					// if this is the last part of the transaction, parse the transaction into human readable text, and display it.
					if (G_io_apdu_buffer[2] == P1_LAST) {
						raw_tx_len = raw_tx_ix;
						raw_tx_ix = 0;

//								// sign the transaction
						size_t raw_tx_len_except_bip44 = raw_tx_len - BIP44_BYTE_LENGTH;
						unsigned char * bip44_in = raw_tx + raw_tx_len_except_bip44;
//								// BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32.
						unsigned int bip44_path[BIP44_PATH_LEN];
						uint32_t i;
						for (i = 0; i < BIP44_PATH_LEN; i++) {
							bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
							bip44_in += 4;
						}

						ed25519_secret_key sk;
						os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip44_path, BIP44_PATH_LEN, sk, NULL);
						ed25519_public_key pk;
						ed25519_publickey(sk, pk);
						ed25519_sign(raw_tx, raw_tx_len_except_bip44, sk, pk, sig);

						//os_memmove(G_io_apdu_buffer, sig, sizeof(sig));
						//tx = sizeof(sig);

						// populate teh uis for account and amount.
						update_tx_address_data();
						update_tx_amount_data();

						// display the UI, starting at the top screen which is "Sign Tx".
						ui_sign();
					}

					if (G_io_apdu_buffer[2] == P1_MORE) {
						// return 0x9000 OK.
						THROW(0x9000);
					} else {
						flags |= IO_ASYNCH_REPLY;
					}
				}
				break;

				// we're asked for the public key.
				case INS_GET_SIGNED_PUBLIC_KEY:
				case INS_GET_PUBLIC_KEY: {
					Timer_Restart();

					if (rx < APDU_HEADER_LENGTH + BIP44_BYTE_LENGTH) {
						THROW(0x6D09);
					}

					/** BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32. */
					unsigned char * bip44_in = G_io_apdu_buffer + APDU_HEADER_LENGTH;

					unsigned int bip44_path[BIP44_PATH_LEN];
					uint32_t i;
					for (i = 0; i < BIP44_PATH_LEN; i++) {
						bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
						bip44_in += 4;
					}

					// https://github.com/orlp/ed25519
					ed25519_secret_key sk;
					os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip44_path, BIP44_PATH_LEN, sk, NULL);
					ed25519_public_key pk;
					ed25519_publickey(sk, pk);

					update_public_key_data(&pk, C_icon_banano);

					refresh_public_key_display();
					refresh_idle_display();

					os_memmove(G_io_apdu_buffer, pk, sizeof(pk));
					tx = sizeof(pk);

//							unsigned char publicKeyData[33];
//						    xrp_compress_public_key(&publicKey, publicKeyData,33);
//							os_memmove(G_io_apdu_buffer, publicKeyData, 33);
//							tx = 33;

					// return 0x9000 OK.
					THROW(0x9000);
				}
				break;

				case INS_GET_BASE_10_ENCODED: {
					Timer_Restart();
					unsigned int in_len = get_apdu_buffer_length();
					unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH;
					if(in_len >= 0xFF) {
						G_io_apdu_buffer[tx++] = 0x0E;
						G_io_apdu_buffer[tx++] = in_len >> 24;
						G_io_apdu_buffer[tx++] = in_len >> 16;
						G_io_apdu_buffer[tx++] = in_len >> 8;
						G_io_apdu_buffer[tx++] = in_len;
						G_io_apdu_buffer[tx++] = 0xE0;
						THROW(0x6D17);
					}
					char out[0x10];
					os_memset(out,0x00,sizeof(out));
					const unsigned int out_length = sizeof(out);
					const bool enable_debug = DEBUG_OUT_ENABLED;
					clearDebug();
					encode_base_10(in, in_len, out,out_length,enable_debug);
					G_io_apdu_buffer[tx++] = 0xFF;
					G_io_apdu_buffer[tx++] = 0xF0;
					G_io_apdu_buffer[tx++] = 0x0F;
					os_memmove(G_io_apdu_buffer + tx, out, out_length);
					tx += out_length;
					G_io_apdu_buffer[tx++] = 0xF0;
					G_io_apdu_buffer[tx++] = 0x0F;
					G_io_apdu_buffer[tx++] = 0xFF;
					THROW(0x9000);
				}
				break;

				case 0xFF:     // return to dashboard
					goto return_to_dashboard;

				// we're asked to do an unknown command
				default:
					// return an error.
					THROW(0x6D00);
					break;
				}
			}
			CATCH_OTHER(e)
			{
				switch (e & 0xF000) {
				case 0x6000:
				case 0x9000:
					sw = e;
					break;
				default:
					sw = 0x6800 | (e & 0x7FF);
					break;
				}
				if(DEBUG_OUT_ENABLED) {
					const unsigned int debug_length = debug_ix;
					os_memmove(G_io_apdu_buffer + tx, debug_out, debug_length);
					tx += debug_length;
				}
				// Unexpected exception => report
				G_io_apdu_buffer[tx] = sw >> 8;
				G_io_apdu_buffer[tx + 1] = sw;
				tx += 2;
			}
			FINALLY
			{
			}
		}
		END_TRY;
	}

return_to_dashboard: return;
}

/** display function */
void io_seproxyhal_display(const bagl_element_t *element) {
	io_seproxyhal_display_default((bagl_element_t *) element);
}

/* io event loop */
unsigned char io_event(unsigned char channel) {
	// nothing done with the event, throw an error on the transport layer if
	// needed

	// can't have more than one tag in the reply, not supported yet.
	switch (G_io_seproxyhal_spi_buffer[0]) {
	case SEPROXYHAL_TAG_FINGER_EVENT:
		Timer_Restart();
		UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
		Timer_Restart();
		UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
		//Timer_Restart();
		if (UX_DISPLAYED()) {
			// perform actions after all screen elements have been displayed
		} else {
			UX_DISPLAYED_EVENT();
		}
		break;

	case SEPROXYHAL_TAG_TICKER_EVENT:
//		UX_REDISPLAY();
		// if(UI_TX_ADDR) {
		//  viewNeedsRefresh = 1;
		// }
		// if(UX_TX_AMT) {
		//  viewNeedsRefresh = 1;
		// }
		Timer_Tick();
		if (viewNeedsRefresh == 1) {
			UX_REDISPLAY();
			viewNeedsRefresh = 0;
		} else {
			if (Timer_Expired()) {
				os_sched_exit(0);
			}
		}

		break;

	// unknown events are acknowledged
	default:
		UX_DEFAULT_EVENT();
		break;
	}

	// close the event if not done previously (by a display or whatever)
	if (!io_seproxyhal_spi_is_status_sent()) {
		io_seproxyhal_general_status();
	}

	// command has been processed, DO NOT reset the current APDU transport
	return 1;
}

/** boot up the app and intialize it */
__attribute__((section(".boot"))) int main(void) {
	// exit critical section
	__asm volatile ("cpsie i");

	raw_tx_ix = 0;
	uiState = UI_IDLE;

	// ensure exception will work as planned
	os_boot();

	UX_INIT();

	BEGIN_TRY
	{
		TRY
		{
			io_seproxyhal_init();

#ifdef LISTEN_BLE
			if (os_seph_features() &
			    SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_BLE) {
				BLE_power(0, NULL);
				// restart IOs
				BLE_power(1, NULL);
			}
#endif

			USB_power(0);
			USB_power(1);

			// init the public key display to "no public key".
			update_no_public_key_data();

			// show idle screen.
			ui_idle();

			// set timer
			Timer_Set();

			// run main event loop.
			banano_main();
		}
		CATCH_OTHER(e)
		{
		}
		FINALLY
		{
		}
	}
	END_TRY;
}
