#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

static const char *rsa_meth_name = "testmethod";
static const RSA_METHOD *org = NULL;
static RSA_METHOD *meth = NULL;
static const char key_file[] = "client.key";
static EVP_PKEY *_pkey = NULL;

static int dmy_pub_enc(int flen, 
	const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_pub_enc(flen, from, to, rsa, padding);
}

#if 0
static int dmy_pri_enc(int flen, const unsigned char *from,
                                   unsigned char *to, RSA *rsa, int padding)
{
	EVP_PKEY_CTX *ctx;
	ENGINE *eng;
	unsigned char *out = to;
	unsigned char *in = from;
	size_t outlen; 
	size_t inlen = flen;
	EVP_PKEY *key = _pkey;
 
	printf("%s %s\n", __func__, rsa->meth->name);
	ctx = EVP_PKEY_CTX_new(key, key->engine);
	if (!ctx) {
		perror("EVP_PKEY_CTX_new:");
		exit (-1);
	}
 	
	if (EVP_PKEY_encrypt_init(ctx) <= 0) {
		perror("EVP_PKEY_encrypt_init:");
		exit (-1);
	}

#if 1 
	if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) {
		perror("EVP_PKEY_set_rsa_padding:");
		exit (-1);
	}
#endif
 	if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
		perror("EVP_PKEY_encrypt1:");
		exit (-1);
	}

	if (EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <= 0) {
		perror("EVP_PKEY_encrypt2:");
		exit (-1);
	}
	printf("%s %s\n", __func__, rsa->meth->name);
}
# else
static int dmy_pri_enc(int flen, const unsigned char *from,
                                   unsigned char *to, RSA *rsa, int padding)
{
	int ret;
	int i;
	char *p = from;
	printf("%s %s\n", __func__, rsa->meth->name);
	for (i = 0; i < 256; i++) {
		printf("0x%x ", (*p & 0xFF));
		p++;
	}
	ret = org->rsa_priv_enc(flen, from, to, rsa, padding);
	printf("\n-----\n");
	p = to;
	for (i = 0; i < 256; i++) {
		printf("0x%x ", (*p & 0xFF));
		p++;
	}
	return ret;
}
#endif

static int dmy_pub_dec(int flen, const unsigned char *from,
                                  unsigned char *to, RSA *rsa, int padding)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_pub_dec(flen, from, to, rsa, padding);
}

static int dmy_pri_dec(int flen, const unsigned char *from,
                                   unsigned char *to, RSA *rsa, int padding)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_priv_dec(flen, from, to, rsa, padding);
}

static int dmy_mod_exp(BIGNUM *r0, const BIGNUM *i, RSA *rsa,
                           BN_CTX *ctx)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_mod_exp(r0, i, rsa, ctx);
}

static int dmy_bn_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
	const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
	printf("%s\n", __func__);
	return org->bn_mod_exp(r, a, p, m, ctx, m_ctx);
}

static int dmy_init(RSA *rsa)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->init(rsa);
}

static int dmy_finish(RSA *rsa)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->finish(rsa);
}

int dmy_sign(int type, const unsigned char *m, unsigned int m_length,
		unsigned char *sigret, unsigned int *siglen, const RSA *rsa)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_sign(type, m, m_length, sigret, siglen, rsa);
}

int dmy_verify(int dtype, const unsigned char *m, unsigned int m_length, 
	const unsigned char *sigbuf, unsigned int siglen, const RSA *rsa)
{
	printf("%s %s\n", __func__, rsa->meth->name);
	return org->rsa_verify(dtype, m, m_length, sigbuf, siglen, rsa);
}

void replace_rsa_method(RSA *rsa)
{
	if (!meth) {
		FILE *f = fopen(key_file, "r");
		if (f == NULL) {
			perror("flopen:");
			exit(-1);
		}

		_pkey = PEM_read_PrivateKey(f, NULL, NULL, NULL);
		if (_pkey == NULL) {
			perror("PEM_read_PrivateKey(client.key):");
			fclose(f);
			exit (-1);
		}
		fclose(f);
		
		org = RSA_get_method(rsa);
		meth = malloc(sizeof(RSA_METHOD));
		if (!meth) {
			perror("malloc:");
			exit(-1);
		}
		memcpy(meth, org, sizeof(RSA_METHOD));
	}
	meth->flags |= RSA_METHOD_FLAG_NO_CHECK;
	meth->name = rsa_meth_name;

	meth->rsa_pub_enc = dmy_pub_enc;
	meth->rsa_priv_enc = dmy_pri_enc;
	meth->rsa_pub_dec = dmy_pub_dec;
	meth->rsa_priv_dec = dmy_pri_dec;
	meth->rsa_mod_exp = dmy_mod_exp;
	meth->bn_mod_exp = dmy_bn_mod_exp;
	meth->init = dmy_init;
	meth->finish = dmy_finish;	
	meth->rsa_sign = dmy_sign;	
	meth->rsa_verify = dmy_verify;	
	
	RSA_set_method(rsa, meth);
//	rsa->flags |= RSA_FLAG_EXT_PKEY;
	return;
}
