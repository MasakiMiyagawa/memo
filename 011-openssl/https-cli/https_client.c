#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <libgen.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>

static char buf[1024*1024] = {0};
static const char crt_file[] = "client.crt";
static const char key_file[] = "client.key";
static const char *service = "https";
static int32_t sock;
static SSL *ssl;
static SSL_CTX *ctx;
static char *host = "localhost";
static char *path = "";

static int ssl_sample_client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
	X509 *_x509 = NULL;
	EVP_PKEY *_pkey = NULL;
	FILE* f = fopen(crt_file, "r");
    	if (f == NULL) {
		perror("fopen:");
		exit(-1);
	}
        
	_x509 = PEM_read_X509(f, NULL, NULL, NULL);
       	if (_x509 == NULL) {
		fclose(f);
		perror("PEM_read_X509:");
		exit (-1);
	}
	
	fclose(f);
	f = NULL;
	*x509 = _x509;

	f = fopen(key_file, "r");
	if (f == NULL) {
		perror("flopen:");
		X509_free(_x509);
		exit(-1);
	}

	_pkey = PEM_read_PrivateKey(f, NULL, NULL, NULL);
	if (_pkey == NULL) {
		perror("PEM_read_PrivateKey:");
		X509_free(_x509);
		fclose(f);
		exit (-1);
	}

	fclose(f);
	f = NULL;
	*pkey = _pkey;
	return 1;
}

static int ssl_sample_verify_callback(int preverify_ok, X509_STORE_CTX *x509)
{
	/*
	 * 独自証明書検証関数のsample実装。
	 * preverify_ok=0:事前の検証には失敗している
	 * preverify_ok=1:事前の検証に成功している
	 * この関数のリターン値により検証結果を上書きできる。
	 * return 0:検証エラー
	 * return 1:検証成功
	 * このサンプル実装ではX509証明書を取得し、署名者名をprintfした後
	 * 検証を強制的に成功にする。
	 */
	X509 *cert = X509_STORE_CTX_get_current_cert(x509);
	X509_print_fp(stdout, cert);
	return 1;
}

static int32_t ssl_sample_get_req(void)
{
    	char msg[100] = {0};
	int read_size = 0;
	sprintf(msg, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
	printf("%s", msg);
	if (0 >= SSL_write(ssl, msg, strlen(msg))) {
		perror("SSL_write:");
		return -1;
	}

	while (1) {
 		read_size = SSL_read(ssl, buf, sizeof(buf));
		if (read_size <= 0) 
			break;

		printf("%s", buf);
	}
	printf("\n");

	return 0;
}

static int32_t ssl_sample_init(void)
{
	int32_t ret = 0;

    	SSL_load_error_strings();
    	SSL_library_init();

	ctx = SSL_CTX_new(TLSv1_2_client_method());
	if (!ctx) {
		perror("SSL_CTX_new:");
		return -1;
	}

	ssl = SSL_new(ctx);
	if (!ssl) {
		perror("SSL_new:");
    		SSL_CTX_free(ctx);
		return -1;
	}
	
	SSL_CTX_set_client_cert_cb(ctx, ssl_sample_client_cert_cb);
	/*
	 * SSL_set_verifyにより独自の署名検証を行うことができる。
	 * 検証は第三引数のコールバックで行う。SSL_VERIFY_PEERは
	 * 署名検証に失敗した場合、SSL_connectをFAITAL ERRORで終了
	 * させる。
	 */
	SSL_set_verify(ssl, SSL_VERIFY_PEER, ssl_sample_verify_callback);
 	if (!SSL_set_fd(ssl, sock)) {
		perror("SSL_set_fd:");
    		SSL_free(ssl);
    		SSL_CTX_free(ctx);
		return -1;
	}

	ret = SSL_connect(ssl);
	switch (ret) {
	case 0:
		printf("SSL error %d\n", SSL_get_error(ssl, ret));
    		SSL_free(ssl);
    		SSL_CTX_free(ctx);
		return -1;
	case 1:
		break;
	default:
		printf("SSL fatal error %d\n", SSL_get_error(ssl, ret));
    		SSL_free(ssl);
    		SSL_CTX_free(ctx);
		return -1;
	}

   	printf("Conntect to %s\n", host);
	return 0;
}

int main(int32_t argc, char *argv[])
{
	struct addrinfo hints = {0};
	struct addrinfo *res;
	int err = 0;

	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
		perror("getaddrinfo:");
		return EXIT_FAILURE;
	}

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
		perror("connect");
		return EXIT_FAILURE;
	}

	if (ssl_sample_init()) {
		return EXIT_FAILURE;
	}
	
	if (ssl_sample_get_req()) {
		return EXIT_FAILURE;
	}

	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(sock);
	return EXIT_SUCCESS;
}
