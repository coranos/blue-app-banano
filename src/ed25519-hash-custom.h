/*
 * MIT License, see root folder for full license.
 */

#ifndef ED25519_HASH_CUSTOM_H
#define ED25519_HASH_CUSTOM_H

//#include "os.h"
//#include "cx.h"
#include "blake2b.h"

typedef struct __blake2b_state ed25519_hash_context;

void ed25519_hash_init(ed25519_hash_context *ctx);
void ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen);
void ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash);
void ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen);

#endif // ED25519_HASH_CUSTOM_H

