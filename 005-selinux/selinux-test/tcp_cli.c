#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	int sock;
	char buf[2048];
	int n;
	char *ip = "127.0.0.1";
	uint16_t port = 8000;
	uint32_t count = 0;

	if (argc > 1) {
		ip = argv[1];
	}

	if (argc > 2) {
		port = atoi(argv[2]);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket:");
		exit(-1);
	}

 	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);
	printf("TCP Connect to %s:%u\n", ip, port);
	if (0 > connect(sock, (struct sockaddr *)&server, sizeof(server))) {
		perror("connect:");
		close(sock);
		exit(-1);
	}

	while (1) {
		usleep(1000 * 1000);
 		memset(buf, 0, sizeof(buf));
		sprintf(buf, "Hello count=%d", count++);
		if (write(sock, buf, strlen(buf)) > 0) {
 			n = read(sock, buf, sizeof(buf));
			if (n < 0) {
				perror("read:");
				continue;
			}
			printf("Echoback:%s\n", buf);		
		}
	}
 	/* socketの終了 */
	close(sock);
	return 0;
}
