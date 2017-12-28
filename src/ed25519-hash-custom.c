#include "ed25519-hash-custom.h"

//void ed25519_hash_init(ed25519_hash_context *ctx) {
//}
//
//void ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen){
//}
//
//void ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash){
//}

void ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen){
//	cx_hash_sha512(in, inlen,hash);
	blake2b(in, inlen,hash, 64);

//	blake2b(hash, 32, in, inlen,NULL,0);
}
