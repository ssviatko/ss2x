#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha2.h"

unsigned char k0[128];
unsigned char kxoripad[128]; // k0 ^ opad
unsigned char kxoropad[128]; // k0 ^ ipad
unsigned char ipadmsg[128]; // ipad + message

void hmacsha2(unsigned char *key, int keylen, unsigned char *msg, int msglen, unsigned char *dig, int hashlen)
{
	sha224_ctx ctx224;
	sha256_ctx ctx256;
	sha384_ctx ctx384;
	sha512_ctx ctx512;

	int i, blocklen;

	// set lengths to equal the internal block length of hash algorithm
	// i.e. 64 bytes for SHA224/SHA256, 128 bytes for SHA384/SHA512
	if (hashlen <= 256)
		blocklen = 64;
	else
		blocklen = 128;

	// determine k0
	memset(k0, 0, 128);
	if (keylen <= blocklen)
	{
		memcpy(k0, key, keylen);
	}
	else if (keylen > blocklen)
	{
		switch (hashlen)
		{
			case 224:
				sha224(key, keylen, k0);
				break;
			case 256:
				sha256(key, keylen, k0);
				break;
			case 384:
				sha384(key, keylen, k0);
				break;
			case 512:
				sha512(key, keylen, k0);
				break;
			default:
				printf("error, unsupported hash length\n");
				exit(-1);
				break;
		}
	}

	// set up padding
	for (i = 0; i < blocklen; i++)
	{
		kxoropad[i] = k0[i] ^ 0x5c;
		kxoripad[i] = k0[i] ^ 0x36;
	}

	switch (hashlen)
	{
		case 224:
			// compute hash for ipad + message
			sha224_init(&ctx224);
			sha224_update(&ctx224, kxoripad, blocklen);
			sha224_update(&ctx224, msg, msglen);
			sha224_final(&ctx224, dig);

			// compute has for opad + hash(ipad + message)
			sha224_init(&ctx224);
			sha224_update(&ctx224, kxoropad, blocklen);
			sha224_update(&ctx224, dig, hashlen / 8);
			sha224_final(&ctx224, dig);
			break;
		case 256:
			// compute hash for ipad + message
			sha256_init(&ctx256);
			sha256_update(&ctx256, kxoripad, blocklen);
			sha256_update(&ctx256, msg, msglen);
			sha256_final(&ctx256, dig);

			// compute has for opad + hash(ipad + message)
			sha256_init(&ctx256);
			sha256_update(&ctx256, kxoropad, blocklen);
			sha256_update(&ctx256, dig, hashlen / 8);
			sha256_final(&ctx256, dig);
			break;
		case 384:
			// compute hash for ipad + message
			sha384_init(&ctx384);
			sha384_update(&ctx384, kxoripad, blocklen);
			sha384_update(&ctx384, msg, msglen);
			sha384_final(&ctx384, dig);

			// compute has for opad + hash(ipad + message)
			sha384_init(&ctx384);
			sha384_update(&ctx384, kxoropad, blocklen);
			sha384_update(&ctx384, dig, hashlen / 8);
			sha384_final(&ctx384, dig);
			break;
		case 512:
			// compute hash for ipad + message
			sha512_init(&ctx512);
			sha512_update(&ctx512, kxoripad, blocklen);
			sha512_update(&ctx512, msg, msglen);
			sha512_final(&ctx512, dig);

			// compute has for opad + hash(ipad + message)
			sha512_init(&ctx512);
			sha512_update(&ctx512, kxoropad, blocklen);
			sha512_update(&ctx512, dig, hashlen / 8);
			sha512_final(&ctx512, dig);
			break;
		default:
			printf("error, unsupported hash length\n");
			exit(-1);
			break;
	}
}

void hmacsha224(unsigned char *key, int keylen, unsigned char *msg, int msglen, unsigned char *dig)
{
	hmacsha2(key, keylen, msg, msglen, dig, 224);
}

void hmacsha256(unsigned char *key, int keylen, unsigned char *msg, int msglen, unsigned char *dig)
{
	hmacsha2(key, keylen, msg, msglen, dig, 256);
}

void hmacsha384(unsigned char *key, int keylen, unsigned char *msg, int msglen, unsigned char *dig)
{
	hmacsha2(key, keylen, msg, msglen, dig, 384);
}

void hmacsha512(unsigned char *key, int keylen, unsigned char *msg, int msglen, unsigned char *dig)
{
	hmacsha2(key, keylen, msg, msglen, dig, 512);
}
