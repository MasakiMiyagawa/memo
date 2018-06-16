#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in addr;
	char buf[2048];
	uint16_t recv_port = 8000;  

	if (argc > 1)
		recv_port = atoi(argv[1]);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(-1);
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(recv_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))) {
		perror("bind");
		close(sock);
		exit(-1);
	}

	printf("UDP Recv from INADDR_ANY:%u\n", recv_port);
	while (1) {
 		recv(sock, buf, sizeof(buf), 0);
 		printf("%s\n", buf);
		usleep(1000);
	}
	close(sock);
	return 0;
}
