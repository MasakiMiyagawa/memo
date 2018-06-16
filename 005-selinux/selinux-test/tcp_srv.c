#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int listen_socket;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	int sock;
	uint16_t listen_port = 8000;  

	if (argc > 1)
		listen_port = atoi(argv[1]);

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0) {
		perror("socket:");
		exit(-1);
	}

	addr.sin_family = AF_INET;
 	addr.sin_port = htons(listen_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr))) {
		perror("bind:");
		close(listen_socket);
		exit(-1);
	}

	listen(listen_socket, 5);
	printf("TCP Listen from INADDR_ANY:%u\n", listen_port);

	len = sizeof(client);
	sock = accept(listen_socket, (struct sockaddr *)&client, &len);
	if (sock < 0) {
		perror("accept:");
		close(listen_socket);
		exit(-1);	
	}
	printf("TCP Accept from %s\n", inet_ntoa(client.sin_addr));
 	while (1) {
		char buf[2048];
		int32_t n;
		
		memset(buf, 0, sizeof(buf));
		n = read(sock, buf, sizeof(buf));
		if (n > 0) {
			write(sock, buf, n);
		}
	}
	close(sock);
	close(listen_socket);
	return 0;
}
