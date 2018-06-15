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
extern int32_t invoke_replace(void);

static char buf[1024*1024] = {0};
static const char *service = "https";
static int32_t sock;
static SSL *ssl;
static SSL_CTX *ctx;
static char *host = "mirrors.edge.kernel.org";
static char *path = "/pub/linux/kernel/v4.x/ChangeLog-4.9.88";
//static char *host = "cdn.kernel.org";
//static char *path = "/pub/linux/kernel/v4.x/linux-4.17.1.tar.xz";

static int ssl_sample_verify_callback(int preverify_ok, X509_STORE_CTX *x509)
{
	X509 *cert = X509_STORE_CTX_get_current_cert(x509);
	printf("%s is called with preverify is %s.\n", __func__,
				preverify_ok ? "SUCCEEDED" : "FAILED");
	printf("CERT:name=%s\n", cert->name);
	return 1;
}

static int32_t ssl_sample_get_req(void)
{
    	char msg[100] = {0};
	int read_size = 0;
	int fd = -1;
	int cnt = 0;
	sprintf(msg, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
	printf("%s", msg);
	if (0 >= SSL_write(ssl, msg, strlen(msg))) {
		perror("SSL_write:");
		return -1;
	}

	fd = open(basename(path), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		perror("open:");
		return -1;
	}

	while (1) {
 		read_size = SSL_read(ssl, buf, sizeof(buf));
		if (read_size <= 0) 
			break;

 		write(fd, buf, read_size);
		printf("#");
	}
	printf("\n");
	close(fd);

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
	//SSL_set_verify(ssl, SSL_VERIFY_NONE, ssl_sample_verify_callback);
	SSL_set_verify(ssl, SSL_VERIFY_PEER, ssl_sample_verify_callback);
	//SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);
	//SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);

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
	struct sockaddr_in server;
	struct addrinfo hints = {0};
	struct addrinfo *res;
	int   port = 443;
	int err = 0;

	if (invoke_replace()) {
		perror("replace is failed");
		return EXIT_FAILURE;
	}

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
