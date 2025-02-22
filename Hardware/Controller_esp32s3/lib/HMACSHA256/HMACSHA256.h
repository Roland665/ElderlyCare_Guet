#pragma once


#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define SHA256_BLOCKLEN  64 //size of message block buffer
#define SHA256_DIGESTLEN 32 //size of digest in uint8_t
#define SHA256_DIGESTINT 8  //size of digest in uint32_t




typedef struct sha256_ctx_t
{
	uint64_t len;                 // processed message length
	uint32_t h[SHA256_DIGESTINT]; // hash state
	uint8_t buf[SHA256_BLOCKLEN]; // message block buffer
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *m, uint32_t mlen);
// resets state: calls sha256_init
void sha256_final(SHA256_CTX *ctx, uint8_t *md);

typedef struct hmac_sha256_ctx_t
{
	uint8_t buf[SHA256_BLOCKLEN]; // key block buffer, not needed after init
	uint32_t h_inner[SHA256_DIGESTINT];
	uint32_t h_outer[SHA256_DIGESTINT];
	SHA256_CTX sha;
} HMAC_SHA256_CTX;

void hmac_sha256_init(HMAC_SHA256_CTX *hmac, const uint8_t *key, uint32_t keylen);
void hmac_sha256_update(HMAC_SHA256_CTX *hmac, const uint8_t *m, uint32_t mlen);
void hmac_sha256_final(HMAC_SHA256_CTX *hmac, uint8_t *md);

void pbkdf2_sha256(HMAC_SHA256_CTX *ctx,const uint8_t *key, uint32_t keylen, const uint8_t *salt, uint32_t saltlen, uint32_t rounds,uint8_t *dk, uint32_t dklen);

void HMACSHA256_encrypt(char *output, uint8_t *input, uint8_t *key);
