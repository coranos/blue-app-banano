#include "ed25519-hash-custom.h"

#define OUTPUT_LENGTH 64

void ed25519_hash_init(ed25519_hash_context *ctx) {
	blake2b_Init(ctx, OUTPUT_LENGTH);
}

void ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen){
	blake2b_Update(ctx, in, inlen);
}

void ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash){
	blake2b_Final(ctx, hash, OUTPUT_LENGTH);
}

void ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen){
	blake2b(in, inlen,hash, OUTPUT_LENGTH);
}
