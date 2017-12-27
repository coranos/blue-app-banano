/*
 * MIT License, see root folder for full license.
 */
#include "neo.h"

/** if true, show a screen with the transaction type. */
#define SHOW_TX_TYPE true

/** if true, show a screen with the transaction length. */
#define SHOW_TX_LEN false

/** if true, show a screen with the transaction version. */
#define SHOW_VERSION false

/** if true, show the tx-type exclusive data, such as coin claims for a Claim Tx */
#define SHOW_EXCLUSIVE_DATA false

/** if true, show number of attributes. */
#define SHOW_NUM_ATTRIBUTES false

/** if true, show number of tx-in coin references. */
#define SHOW_NUM_COIN_REFERENCES false

/** if true, show number of output transactions. */
#define SHOW_NUM_TX_OUTS false

/** if true, show tx-out values in hex as well as decimal. */
#define SHOW_VALUE_HEX false

/** if true, show script hash screen as well as address screen */
#define SHOW_SCRIPT_HASH false

/**
 * each CoinReference has two fields:
 *  UInt256 PrevHash = 32 bytes.
 *  ushort PrevIndex = 2 bytes.
 */
#define COIN_REFERENCES_LEN (32 + 2)

/** length of tx.output.value */
#define VALUE_LEN 8

/** length of tx.output.asset_id */
#define ASSET_ID_LEN 32

/** length of tx.output.script_hash */
#define SCRIPT_HASH_LEN 20

/** length of the checksum used to convert a tx.output.script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

/** length of a tx.output Address, after Base58 encoding. */
#define ADDRESS_BASE58_LEN 34

/** length of a tx.output Address before encoding, which is the length of <address_version>+<script_hash>+<checksum> */
#define ADDRESS_LEN (1 + SCRIPT_HASH_LEN + SCRIPT_HASH_CHECKSUM_LEN)

/** the current version of the address field */
#define ADDRESS_VERSION 23

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32

/** the position of the decimal point, 8 characters in from the right side */
#define DECIMAL_PLACE_OFFSET 8

/**
 * transaction types.
 *
 * Currently only Claim and Contract are tested, as they are the only ones supported by the current wallets.
 */
enum TX_TYPE {
	TX_MINER = 0x00, TX_ISSUE = 0x01, TX_CLAIM = 0x02, TX_ENROLL = 0x20, TX_REGISTER = 0x40, TX_CONTRACT = 0x80, TX_PUBLISH = 0xD0, TX_INVOKE = 0xD1
};

/**
 * transaction attributes.
 *
 * currently there's no support in wallets for adding attributes to a contract, but the types are as listed below.
 */
enum TransactionAttributeUsage {
	CONTRACT_HASH = 0x00,

	ECDH02 = 0x02,
	ECDH03 = 0x03,

	SCRIPT = 0x20,

	VOTE = 0x30,

	DESCRIPTION_URL = 0x81,
	DESCRIPTION = 0x90,

	HASH1 = 0xa1,
	HASH2 = 0xa2,
	HASH3 = 0xa3,
	HASH4 = 0xa4,
	HASH5 = 0xa5,
	HASH6 = 0xa6,
	HASH7 = 0xa7,
	HASH8 = 0xa8,
	HASH9 = 0xa9,
	HASH10 = 0xaa,
	HASH11 = 0xab,
	HASH12 = 0xac,
	HASH13 = 0xad,
	HASH14 = 0xae,
	HASH15 = 0xaf,

	REMARK = 0xf0,
	REMARK1 = 0xf1,
	REMARK2 = 0xf2,
	REMARK3 = 0xf3,
	REMARK4 = 0xf4,
	REMARK5 = 0xf5,
	REMARK6 = 0xf6,
	REMARK7 = 0xf7,
	REMARK8 = 0xf8,
	REMARK9 = 0xf9,
	REMARK10 = 0xfa,
	REMARK11 = 0xfb,
	REMARK12 = 0xfc,
	REMARK13 = 0xfd,
	REMARK14 = 0xfe,
	REMARK15 = 0xff
};

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** #### Asset IDs #### */
/** currently only NEO and GAS are supported, alll others show up as UNKNOWN */

/** NEO's asset id. */
static const char NEO_ASSET_ID[] = "C56F33FC6ECFCD0C225C4AB356FEE59390AF8560BE0E930FAEBE74A6DAFF7C9B";

/** GAS's asset id. */
static const char GAS_ASSET_ID[] = "602C79718B16E442DE58778E148D0B1084E3B2DFFD5DE6B7B16CEE7969282DE7";

/** #### End Of Asset IDs #### */

/** NEO asset's label. */
static const char TXT_ASSET_NEO[] = "NEO";

/** GAS asset's label. */
static const char TXT_ASSET_GAS[] = "GAS";

/** default asset label.*/
static const char TXT_ASSET_UNKNOWN[] = "UNKNOWN";

/** text to display if an asset's base-10 encoded value is too low to display */
static const char TXT_LOW_VALUE[] = "Low Value";

/** a period, for displaying the decimal point. */
static const char TXT_PERIOD[] = ".";

/** Version label */
static const char TXT_VERSION[] = "Version";

/** Label when displaying the number of claims. */
static const char TXT_CLAIMS[] = "Num Claims";

/** Label when displaying the number of attributes. */
static const char TXT_NUM_ATTR[] = "Num Attr";

/** Label when displaying the number of transaction inputs. */
static const char TXT_NUM_TXIN[] = "Num Tx In";

/** Label when displaying the number of transaction outputs. */
static const char TXT_NUM_TXOUT[] = "Num Tx Out";

/** Label when displaying a Miner transaction */
static const char TX_MINER_NM[] = "Miner Tx";

/** Label when displaying a Issue transaction */
static const char TX_ISSUE_NM[] = "Issue Tx";

/** Label when displaying a Claim transaction */
static const char TX_CLAIM_NM[] = "Claim Tx";

/** Label when displaying a Enroll transaction */
static const char TX_ENROLL_NM[] = "Enroll Tx";

/** Label when displaying a Register transaction */
static const char TX_REGISTER_NM[] = "Register Tx";

/** Label when displaying a Contract transaction */
static const char TX_CONTRACT_NM[] = "Contract Tx";

/** Label when displaying a Publish transaction */
static const char TX_PUBLISH_NM[] = "Publish Tx";

/** Label when displaying a Invoke transaction */
static const char TX_INVOKE_NM[] = "Invoke Tx";

/** array of capital letter hex values */
static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** array of base58 aplhabet letters */
static const char BASE_58_ALPHABET[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z' };

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/** skips the given number of bytes in the transaction */
static void skip_raw_tx(const unsigned int tx_skip);

/** returns the number of bytes in the next variable byte record, called prior to reading a variable byte record to know how many bytes to read. */
static unsigned char next_raw_tx_varbytes_num();

/** reads a set of bytes into the array pointed to by the arr parameter, reads as many bytes as the length parameter specifies. */
static void next_raw_tx_arr(unsigned char * arr, const unsigned int length);

/** reads the next byte out of the transaction, or throws an error if there are no more bytes left. */
static unsigned char next_raw_tx();

/** returns the minimum of i0 and i1 */
static unsigned int min(const unsigned int i0, const unsigned int i1);

/** reads bytes out of src, converts each byte into two hex characters, and writes the hex characters into dest. Only converts enough characters to fill dest_len. */
static void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len);

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_10(const void *in, unsigned int in_length, char *out, unsigned int out_length) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into base-58, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_58(const void *in, unsigned int length, char *out, unsigned int maxoutlen) {
	return encode_base_x(BASE_58_ALPHABET, sizeof(BASE_58_ALPHABET), in, length, out, maxoutlen);
}

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, unsigned int alphabet_len, const void * in, unsigned int in_length, char * out,
		unsigned int out_length) {
	char tmp[164];
	char buffer[164];
	unsigned char j;
	unsigned char startAt;
	unsigned char zeroCount = 0;
	if (in_length > sizeof(tmp)) {
		hashTainted = 1;
		THROW(INVALID_PARAMETER);
	}
	os_memmove(tmp, in, in_length);
	while ((zeroCount < in_length) && (tmp[zeroCount] == 0)) {
		++zeroCount;
	}
	j = 2 * in_length;
	startAt = zeroCount;
	while (startAt < in_length) {
		unsigned short remainder = 0;
		unsigned char divLoop;
		for (divLoop = startAt; divLoop < in_length; divLoop++) {
			unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
			unsigned short tmpDiv = remainder * 256 + digit256;
			tmp[divLoop] = (unsigned char) (tmpDiv / alphabet_len);
			remainder = (tmpDiv % alphabet_len);
		}
		if (tmp[startAt] == 0) {
			++startAt;
		}
		buffer[--j] = *(alphabet + remainder);
	}
	while ((j < (2 * in_length)) && (buffer[j] == *(alphabet + 0))) {
		++j;
	}
	while (zeroCount-- > 0) {
		buffer[--j] = *(alphabet + 0);
	}
	in_length = 2 * in_length - j;
	if (out_length < in_length) {
//		THROW(EXCEPTION_OVERFLOW);
		os_memmove(out, (buffer + j), out_length);
		return out_length;
	} else {
		os_memmove(out, (buffer + j), in_length);
		return in_length;
	}
}

/** converts a value to base10 with a decimal point at DECIMAL_PLACE_OFFSET, which should be 100,000,000 or 100 million, thus the suffix 100m */
static void to_base10_100m(char * dest, const unsigned char * value, const unsigned int dest_len) {
	// reverse the array
	unsigned char reverse_value[VALUE_LEN];
	for (int ix = 0; ix < VALUE_LEN; ix++) {
		reverse_value[ix] = *(value + ((VALUE_LEN - 1) - ix));
	}

	// encode in base10
	char base10_buffer[MAX_TX_TEXT_WIDTH];
	unsigned int buffer_len = encode_base_10(reverse_value, VALUE_LEN, base10_buffer, MAX_TX_TEXT_WIDTH);

	// place the decimal place.
	unsigned int dec_place_ix = buffer_len - DECIMAL_PLACE_OFFSET;
	if (buffer_len < DECIMAL_PLACE_OFFSET) {
		os_memmove(dest, TXT_LOW_VALUE, sizeof(TXT_LOW_VALUE));
	} else {
		os_memmove(dest + dec_place_ix, TXT_PERIOD, sizeof(TXT_PERIOD));
		os_memmove(dest, base10_buffer, dec_place_ix);
		os_memmove(dest + dec_place_ix + 1, base10_buffer + dec_place_ix, buffer_len - dec_place_ix);
	}
}

/** converts a NEO scripthas to a NEO address by adding a checksum and encoding in base58 */
static void to_address(char * dest, unsigned char * script_hash, unsigned int dest_len) {
	static cx_sha256_t address_hash;
	unsigned char address_hash_result_0[SHA256_HASH_LEN];
	unsigned char address_hash_result_1[SHA256_HASH_LEN];

	// concatenate the ADDRESS_VERSION and the address.
	unsigned char address[ADDRESS_LEN];
	address[0] = ADDRESS_VERSION;
	os_memmove(address + 1, script_hash, SCRIPT_HASH_LEN);

	// do a sha256 hash of the address twice.
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address, SCRIPT_HASH_LEN + 1, address_hash_result_0);
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address_hash_result_0, SHA256_HASH_LEN, address_hash_result_1);

	// add the first bytes of the hash as a checksum at the end of the address.
	os_memmove(address + 1 + SCRIPT_HASH_LEN, address_hash_result_1, SCRIPT_HASH_CHECKSUM_LEN);

	// encode the version + address + cehcksum in base58
	encode_base_58(address, ADDRESS_LEN, dest, dest_len);
}

/** converts a byte array in src to a hex array in dest, using only dest_len bytes of dest before stopping. */
static void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len) {
	for (unsigned int src_ix = 0, dest_ix = 0; dest_ix < dest_len; src_ix++, dest_ix += 2) {
		unsigned char src_c = *(src + src_ix);
		unsigned char nibble0 = (src_c >> 4) & 0xF;
		unsigned char nibble1 = src_c & 0xF;

		*(dest + dest_ix + 0) = HEX_CAP[nibble0];
		*(dest + dest_ix + 1) = HEX_CAP[nibble1];
	}
}

/** returns true if the byte array in asset_id matches the hex in asset_id_hex. */
static bool is_asset_id(const unsigned char * asset_id, const char * asset_id_hex) {
	for (int asset_id_ix = ASSET_ID_LEN - 1, asset_id_hex_ix = 0; asset_id_ix >= 0; asset_id_ix--, asset_id_hex_ix += 2) {
		unsigned char asset_id_c = *(asset_id + asset_id_ix);
		unsigned char nibble0 = (asset_id_c >> 4) & 0xF;
		unsigned char nibble1 = asset_id_c & 0xF;

		if (*(asset_id_hex + asset_id_hex_ix + 0) != HEX_CAP[nibble0]) {
			return false;
		}
		if (*(asset_id_hex + asset_id_hex_ix + 1) != HEX_CAP[nibble1]) {
			return false;
		}
	}
	return true;
}

/** returns the minimum of two ints. */
static unsigned int min(unsigned int i0, unsigned int i1) {
	if (i0 < i1) {
		return i0;
	} else {
		return i1;
	}
}

/** skips the given number of bytes in the raw_tx buffer. If this goes off the end of the buffer, throw an error. */
static void skip_raw_tx(unsigned int tx_skip) {
	raw_tx_ix += tx_skip;
	if (raw_tx_ix >= raw_tx_len) {
		hashTainted = 1;
		THROW(0x6D03);
	}
}

/** returns the number of bytes to read for the next varbytes array.
 *  Currently throws an error if the encoded value should be over 253,
 *   which should never happen in this use case of a varbyte array
 */
static unsigned char next_raw_tx_varbytes_num() {
	unsigned char num = next_raw_tx();
	switch (num) {
	case 0xFD:
	case 0xFE:
	case 0xFF:
		hashTainted = 1;
		THROW(0x6D04);
		break;
	default:
		break;
	}
	return num;
}

/** fills the array in arr with the given number of bytes from raw_tx. */
static void next_raw_tx_arr(unsigned char * arr, unsigned int length) {
	for (unsigned int ix = 0; ix < length; ix++) {
		*(arr + ix) = next_raw_tx();
	}
}

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
static unsigned char next_raw_tx() {
	if (raw_tx_ix < raw_tx_len) {
		unsigned char retval = raw_tx[raw_tx_ix];
		raw_tx_ix += 1;
		return retval;
	} else {
		hashTainted = 1;
		THROW(0x6D05);
		return 0;
	}
}

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc() {
	unsigned int scr_ix = 0;
	char hex_buffer[MAX_TX_TEXT_WIDTH];
	unsigned int hex_buffer_len = 0;

	enum TX_TYPE trans_type = next_raw_tx();
	if (SHOW_TX_TYPE) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			// add transaction type screen.
			os_memmove(tx_desc[scr_ix][0], TXT_BLANK, sizeof(TXT_BLANK));
			switch (trans_type) {
			case TX_MINER:
				os_memmove(tx_desc[scr_ix][1], TX_MINER_NM, sizeof(TX_MINER_NM));
				break;

			case TX_ISSUE:
				os_memmove(tx_desc[scr_ix][1], TX_ISSUE_NM, sizeof(TX_ISSUE_NM));
				break;

			case TX_CLAIM:
				os_memmove(tx_desc[scr_ix][1], TX_CLAIM_NM, sizeof(TX_CLAIM_NM));
				break;

			case TX_ENROLL:
				os_memmove(tx_desc[scr_ix][1], TX_ENROLL_NM, sizeof(TX_ENROLL_NM));
				break;

			case TX_REGISTER:
				os_memmove(tx_desc[scr_ix][1], TX_REGISTER_NM, sizeof(TX_REGISTER_NM));
				break;

			case TX_CONTRACT:
				os_memmove(tx_desc[scr_ix][1], TX_CONTRACT_NM, sizeof(TX_CONTRACT_NM));
				break;

			case TX_PUBLISH:
				os_memmove(tx_desc[scr_ix][1], TX_PUBLISH_NM, sizeof(TX_PUBLISH_NM));
				break;

			case TX_INVOKE:
				os_memmove(tx_desc[scr_ix][1], TX_INVOKE_NM, sizeof(TX_INVOKE_NM));
				break;

			default:
				hashTainted = 1;
				THROW(0x6D06);

			}

			if (SHOW_TX_LEN) {
				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_len)) * 2;
				to_hex(hex_buffer, (unsigned char *) &raw_tx_len, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			} else {
				os_memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));
			}
			scr_ix++;
		}
	}

	// the version screen.
	unsigned char version = next_raw_tx();
	if (SHOW_VERSION) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memmove(tx_desc[scr_ix][0], TXT_VERSION, sizeof(TXT_VERSION));

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(version)) * 2;
			to_hex(hex_buffer, &version, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_ix)) * 2;
			to_hex(hex_buffer, (unsigned char *) &raw_tx_ix, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			scr_ix++;
		}
	}

	// the exclusive data screen.
	switch (trans_type) {
	case TX_CLAIM: {
		unsigned char num_coin_claims = next_raw_tx_varbytes_num();
		if (SHOW_EXCLUSIVE_DATA) {
			if (scr_ix < MAX_TX_TEXT_SCREENS) {
				os_memmove(tx_desc[scr_ix][0], TXT_CLAIMS, sizeof(TXT_CLAIMS));

				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(num_coin_claims)) * 2;
				to_hex(hex_buffer, &num_coin_claims, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_ix)) * 2;
				to_hex(hex_buffer, (unsigned char *) &raw_tx_ix, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
				scr_ix++;
			}
		}
		skip_raw_tx(num_coin_claims * COIN_REFERENCES_LEN);
	}
		break;
	case TX_INVOKE: {
		unsigned char script_len = next_raw_tx_varbytes_num();
		skip_raw_tx(script_len);
		if(version >= 1) {
			//UInt64.SIZE = 8
			skip_raw_tx(8);
		}
	}
		break;
	default:
		break;
	}

	//  attributes screen.
	unsigned char num_attr = next_raw_tx_varbytes_num();
	if (SHOW_NUM_ATTRIBUTES) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memmove(tx_desc[scr_ix][0], TXT_NUM_ATTR, sizeof(TXT_NUM_ATTR));

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(num_attr)) * 2;
			to_hex(hex_buffer, &num_attr, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_ix)) * 2;
			to_hex(hex_buffer, (unsigned char *) &raw_tx_ix, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			scr_ix++;
		}
	}

	for (int attr_ix = 0; attr_ix < num_attr; attr_ix++) {
		enum TransactionAttributeUsage attr_usage = next_raw_tx();
		switch (attr_usage) {
		case CONTRACT_HASH:
		case VOTE:
		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
		case HASH7:
		case HASH8:
		case HASH9:
		case HASH10:
		case HASH11:
		case HASH12:
		case HASH13:
		case HASH14:
		case HASH15:
			skip_raw_tx(32);
			break;

		case ECDH02:
		case ECDH03:
			skip_raw_tx(32);
			break;

		case SCRIPT:
			skip_raw_tx(20);
			break;

		case DESCRIPTION_URL:
			skip_raw_tx(next_raw_tx());
			break;

		case DESCRIPTION:
		case REMARK:
			skip_raw_tx(next_raw_tx_varbytes_num());
			break;

		default:
			hashTainted = 1;
			THROW(0x6D07);
		}
	}

	// Coin Reference screen.
	unsigned char num_coin_references = next_raw_tx_varbytes_num();
	if (SHOW_NUM_COIN_REFERENCES) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memmove(tx_desc[scr_ix][0], TXT_NUM_TXIN, sizeof(TXT_NUM_TXIN));

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(num_coin_references)) * 2;
			to_hex(hex_buffer, &num_coin_references, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_ix)) * 2;
			to_hex(hex_buffer, (unsigned char *) &raw_tx_ix, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			scr_ix++;
		}
	}
	skip_raw_tx(num_coin_references * COIN_REFERENCES_LEN);

	// transaction output screen.
	unsigned char num_tx_outs = next_raw_tx_varbytes_num();
	if (SHOW_NUM_TX_OUTS) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memmove(tx_desc[scr_ix][0], TXT_NUM_TXOUT, sizeof(TXT_NUM_TXOUT));

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(num_tx_outs)) * 2;
			to_hex(hex_buffer, &num_tx_outs, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, sizeof(raw_tx_ix)) * 2;
			to_hex(hex_buffer, (unsigned char *) &raw_tx_ix, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			scr_ix++;
		}
	}

	unsigned char asset_id[ASSET_ID_LEN];
	unsigned char value[VALUE_LEN];
	unsigned char script_hash[SCRIPT_HASH_LEN];
	unsigned int script_hash_len0 = 6;
	unsigned int script_hash_len1 = 7;
	unsigned int script_hash_len2 = 7;
	unsigned char * script_hash0 = script_hash;
	unsigned char * script_hash1 = script_hash + script_hash_len0;
	unsigned char * script_hash2 = script_hash + script_hash_len0 + script_hash_len1;

	char address_base58[ADDRESS_BASE58_LEN];
	unsigned int address_base58_len_0 = 11;
	unsigned int address_base58_len_1 = 11;
	unsigned int address_base58_len_2 = 12;
	char * address_base58_0 = address_base58;
	char * address_base58_1 = address_base58 + address_base58_len_0;
	char * address_base58_2 = address_base58 + address_base58_len_0 + address_base58_len_1;

	for (unsigned int ix = 0; ix < num_tx_outs; ix++) {
		next_raw_tx_arr(asset_id, ASSET_ID_LEN);
		next_raw_tx_arr(value, VALUE_LEN);
		next_raw_tx_arr(script_hash, SCRIPT_HASH_LEN);

		to_address(address_base58, script_hash, ADDRESS_BASE58_LEN);

		// asset_id and value screen
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
			// asset id
			if (is_asset_id(asset_id, NEO_ASSET_ID)) {
				os_memmove(tx_desc[scr_ix][0], TXT_ASSET_NEO, sizeof(TXT_ASSET_NEO));
			} else if (is_asset_id(asset_id, GAS_ASSET_ID)) {
				os_memmove(tx_desc[scr_ix][0], TXT_ASSET_GAS, sizeof(TXT_ASSET_GAS));
			} else {
				os_memmove(tx_desc[scr_ix][0], TXT_ASSET_UNKNOWN, sizeof(TXT_ASSET_UNKNOWN));
			}

			// value, base 10.
			to_base10_100m(tx_desc[scr_ix][1], value, MAX_TX_TEXT_WIDTH);

			// value, base 16.
			if (SHOW_VALUE_HEX) {
				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, VALUE_LEN) * 2;
				to_hex(hex_buffer, value, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);
			} else {
				os_memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));
			}
			scr_ix++;
		}

		// script hash screen
		if (SHOW_SCRIPT_HASH) {
			if (scr_ix < MAX_TX_TEXT_SCREENS) {
				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, script_hash_len0) * 2;
				to_hex(hex_buffer, script_hash0, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][0], hex_buffer, hex_buffer_len);

				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, script_hash_len1) * 2;
				to_hex(hex_buffer, script_hash1, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

				hex_buffer_len = min(MAX_HEX_BUFFER_LEN, script_hash_len2) * 2;
				to_hex(hex_buffer, script_hash2, hex_buffer_len);
				os_memmove(tx_desc[scr_ix][2], hex_buffer, hex_buffer_len);

				scr_ix++;
			}
		}

		// address screen
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
			os_memmove(tx_desc[scr_ix][0], address_base58_0, address_base58_len_0);
			os_memmove(tx_desc[scr_ix][1], address_base58_1, address_base58_len_1);
			os_memmove(tx_desc[scr_ix][2], address_base58_2, address_base58_len_2);

			scr_ix++;
		}
	}

	max_scr_ix = scr_ix;

	os_memmove(curr_tx_desc, tx_desc[curr_scr_ix], CURR_TX_DESC_LEN);

	return 1;
}
