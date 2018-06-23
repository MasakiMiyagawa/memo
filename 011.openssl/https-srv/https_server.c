#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

static SSL_CTX *ctx = NULL;
static const char crt_file[] = "server.crt";
static const char key_file[] = "server.key";
static const char body[] = "hello world";
static const char header[] = "HTTP/1.1 200 OK\r\nContent-Type:\
	 text/html\r\nContent-Length: 11\r\nConnection: Close\r\n";
static int32_t server = -1;

static void simple_https_cleanup(void)
{
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = NULL;
	}

	if (server >= 0) {
		close(server);
		server = -1;
	}

	return;
}

static void simple_https_openssl_init(void)
{
	SSL_load_error_strings();
	SSL_library_init();
}

static void simple_https_create_context(void)
{
	ctx = SSL_CTX_new(TLSv1_2_server_method());
	if (!ctx) {
		perror("SSL_CTX_new:");
		exit(-1);
	}

	return;
}

static void simple_https_specify_cert(void)
{
	int32_t ret = 0;

	ret = SSL_CTX_use_certificate_file(ctx, crt_file, SSL_FILETYPE_PEM);
	if (ret != 1) {
		perror("SSL_CTX_use_certificate_file:");
		simple_https_cleanup();
		exit(-1);
	}

	ret = SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM);
	if (ret != 1) {
		perror("SSL_CTX_use_PrivateKey_file:");
		simple_https_cleanup();
		exit(-1);
	}

	return;
}

static void simple_https_create_socket(int32_t port)
{
	struct sockaddr_in addr = {0};

	server = socket(PF_INET, SOCK_STREAM, 0);
	if (server < 0) {
		perror("socket:");
		simple_https_cleanup();
		exit(-1);
	}
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(server, (struct sockaddr*)&addr, sizeof(addr))) {
		perror("bind:");
		simple_https_cleanup();
		exit(-1);
	}
}

static void simple_https_service(void)
{
	int32_t client = -1;
	struct sockaddr_in addr = {0};
	socklen_t size = sizeof(struct sockaddr_in);
	SSL *ssl = NULL;
	char req[1024] = {0};
	char res[1024] = {0};
	int32_t ret;
	
	while (1) {
		memset(req, 0, sizeof(req));
		memset(res, 0, sizeof(res));

		client = accept(server, (struct sockaddr*)&addr, &size);
		if (client < 0) {
			perror("accept:");
			simple_https_cleanup();
			exit(-1);
		}
	
		printf("Connected from: %s:%d\n", 
			inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, client);

		ret = SSL_accept(ssl);
		if (ret > 0) {
			ret = SSL_read(ssl, req, sizeof(req));
			if (ret <= 0) {
				fprintf(stderr, "SSL_read: error %d\n", 
					SSL_get_error(ssl, ret));
				goto error;
			}
			printf("%s\n", req);
			snprintf(res, sizeof(res), "%s\r\n%s", header, body);
				
			ret = SSL_write(ssl, res, strlen(res));
			if (ret <= 0) {
				fprintf(stderr, "SSL_write: error %d\n", 
					SSL_get_error(ssl, ret));
				goto error;
			}
		} else {
			fprintf(stderr, "SSL_accept: error %d\n",
				 SSL_get_error(ssl, ret));
			goto error;
		}

		SSL_free(ssl);
		ssl = NULL;

		client = SSL_get_fd(ssl);
		close(client);
		client = -1;
	}
error:
	if (ssl)
		SSL_free(ssl);

	if (client >= 0) 
		close(client);
		
	simple_https_cleanup();
	exit(-1);
}

int main(int argc, char *argv[])
{
	int32_t port = 9999;

	if (argc > 1)
		port = atoi(argv[1]);

	simple_https_openssl_init();
	simple_https_create_context();
	simple_https_specify_cert();
	simple_https_create_socket(port);

	if (listen(server, 10)) {
		simple_https_cleanup();
		exit(-1);
	}

	simple_https_service();
	simple_https_cleanup();
	return 0;
}
