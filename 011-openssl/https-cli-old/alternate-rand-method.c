#include <openssl/rand.h>
#include <stdint.h>
#include <stdio.h>

static const RAND_METHOD *original = NULL;
static RAND_METHOD sample;

static void sample_seed(const void *buf, int num)
{
	printf("%s is called num=%d\n.", __func__, num);
	original->seed(buf, num);
}

static int sample_bytes(unsigned char *buf, int num)
{
	printf("%s is called num=%d\n.", __func__, num);
	return original->bytes(buf, num);
}

static void sample_cleanup(void)
{
	printf("%s is called\n.", __func__);
	original->cleanup();
}

static void sample_add(const void *buf, int num, double entropy)
{
	printf("%s is called num=%d entropy=%lf\n.", __func__, num, entropy);
	original->add(buf, num, entropy);
}

static int sample_pseudorand(unsigned char *buf, int num)
{
	printf("%s is called num=%d\n.", __func__, num);
	return original->pseudorand(buf, num);
}

static int sample_status(void)
{
	printf("%s is called\n.", __func__);
	original->status();
}

static int32_t replace_nand_method_sample(const RAND_METHOD *replace)
{
	original = RAND_get_rand_method();
	if (!original)
		return -1;

	RAND_set_rand_method(replace);
	return 0;
}

int32_t invoke_replace(void)
{
	sample.seed = sample_seed;
	sample.bytes = sample_bytes;
	sample.cleanup = sample_cleanup;
	sample.add = sample_add;
	sample.pseudorand = sample_pseudorand;
	sample.status = sample_status;
	return replace_nand_method_sample(&sample);
}
