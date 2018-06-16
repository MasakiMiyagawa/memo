#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in addr;
	char *ip = "127.0.0.1";
	uint16_t port = 8000;
	char *msg = "Hello";

	if (argc > 1) {
		ip = argv[1];
	}

	if (argc > 2) {
		port = atoi(argv[2]);
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket:");
		exit(-1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	
	printf("UDP SEND to %s:%u\n", ip, port);
	while (1) {
		int32_t err = 0;	
		err = sendto(sock, msg, strlen(msg), 
			MSG_CONFIRM|MSG_DONTWAIT, 
			(struct sockaddr *)&addr, sizeof(addr));
		if (err < 0) {
			perror("sendto:");
		} else {
			printf("send:%s\n", msg);
		}
		usleep(1000 * 1000);
	}
	
	close(sock);
	return 0;
}
