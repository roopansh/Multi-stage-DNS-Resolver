#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define QUEUE_LEN 3

int main(int argc, char const *argv[]) {
	// Variable Declarations
	int dns_server_fd, new_socket, message_recvd;
	struct sockaddr_in dns_address, client_address;
	int opt = 1;
	int addrlen = sizeof(dns_address);
	char buffer_in[1024] = {0}, buffer_out[1024] = {0} ;
	FILE *database_fp;
	int found_flag;
	char ip_address[1023], domain_name[1023];

	// Creating socket file descriptor for listening at the DNS Server
	if ((dns_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket Failure : ");
		exit(EXIT_FAILURE);
	}

	// Setting options to allow for reuse of the port and dns address
	if (setsockopt(dns_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("Socket Options Failure : ");
		exit(EXIT_FAILURE);
	}

	dns_address.sin_family = AF_INET;
	dns_address.sin_addr.s_addr = INADDR_ANY;
	dns_address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(dns_server_fd, (struct sockaddr *)&dns_address, sizeof(dns_address))<0) {
		perror("Bind Failure : ");
		exit(EXIT_FAILURE);
	}

	// Listening on the created socket
	if (listen(dns_server_fd, QUEUE_LEN) < 0) {
		perror("Socket Listen Failure : ");
		exit(EXIT_FAILURE);
	}

	while(1) {
		if ((new_socket = accept(dns_server_fd, (struct sockaddr *)&client_address, (socklen_t*)&client_address))<0) {
			perror("Socket Accept Failure : ");
			exit(EXIT_FAILURE);
		}

		// empty the buffers
		memset(&buffer_in, '\0', 1024);
		memset(&buffer_out, '\0', 1024);

		message_recvd = read( new_socket , buffer_in, 1024);
		printf("Request :\tType-%c,\tMessage-%s\n",buffer_in[0], buffer_in+1 );

		switch(buffer_in[0]) {
			// Recvd message contains Domain name, requesting IP address
			case '1':
			found_flag = 0;
			database_fp = fopen("database.csv","r");
			while(fscanf(database_fp, "%[^,],%[^\n]\n", domain_name, ip_address) != -1){
				if(strcmp(domain_name, buffer_in+1) == 0){
					found_flag = 1;
					buffer_out[0] = '3';
					strcat(buffer_out, ip_address);
					send(new_socket, buffer_out, strlen(buffer_out), 0);
					break;
				}
			}
			fclose(database_fp);
			if (found_flag == 0) {
				strcpy(buffer_out, "4Entry not found in the database");
				send(new_socket, buffer_out, strlen(buffer_out), 0);
			}
			break;

			// Recvd message contains IP address, requesting Domain name
			case '2':
			found_flag = 0;
			database_fp = fopen("database.csv","r");
			while(fscanf(database_fp, "%[^,],%[^\n]\n", domain_name, ip_address) != -1){
				if(strcmp(ip_address, buffer_in+1) == 0){
					found_flag = 1;
					buffer_out[0] = '3';
					strcat(buffer_out, domain_name);
					send(new_socket, buffer_out, strlen(buffer_out), 0);
					break;
				}
			}
			fclose(database_fp);
			if (found_flag == 0){
				strcpy(buffer_out, "4Entry not found in the database");
				send(new_socket, buffer_out, strlen(buffer_out), 0);
			}
			break;
			default:
				// If the message is incorrect format, then send error type message.
			strcpy(buffer_out, "4Wrong Packet Format");
			send(new_socket, buffer_out, strlen(buffer_out) , 0 );
			fprintf(stderr, "%s\n", "Wrong Packet Format Recvd");
			break;
		}

		printf("Response :\tType-%c,\tMessage-%s\n", buffer_out[0], buffer_out+1);
		printf("%s\n", "______________________________________________________________________________");
		close(new_socket);
	}
	return 0;
}
