//most of the codes are from 'Beej's guide' and 'GeeksforGeeks'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#define BUF_SIZE 100

int main(int argc, char **argv){
	if(argc != 2){ //bad argument
		printf("Usage: server <UDP listen port>\n");
	}
	//Declaring variables used in this program
	int port = atoi(argv[1]); //assigning port
	printf("port: %d\n", port);
	int sockfd; //listen on sockfd
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	char buf[BUF_SIZE];
	socklen_t len;
	int n;

	//Creating socket file descriptor
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		printf("Socket ERROR!\n");
		exit(1);
	}
	printf("sockfd: %d\n", sockfd); //checkpoint 1

	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	//Filling server information
	server_addr.sin_family = AF_INET; //IPv4
	server_addr.sin_addr.s_addr = (INADDR_ANY);
	server_addr.sin_port = htons(port);

	//Bind the socket with the server address
	if(bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
		printf("Bind ERROR!\n");
		exit(1);
	}
	printf("bind done\n"); //checkpoint 2

	len = sizeof(client_addr); //length of client address

	//Receiving message from the client
	//n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len);
	//printf("checkpoint\n");
	//buf[n] = '\0';

	//dummy recieve
	//recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len);
	//printf("recieved message: %s\n", buf);

	if(recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len) == -1){
		printf("recvfrom ERROR!\n");
		exit(1);
	}
	printf("recvfrom done\n");

	//Sending message back to client
	//if the message is "ftp", reply with a message "yes" to the client
	if(strcmp(buf, "ftp") == 0){ 
		if(sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr*)&client_addr, len) == -1){
			printf("sendto ERROR!\n");
			exit(1);
		}
		else{
			printf("SUCCESS\n");
		}
	}
	//else, reply with a message "no" to the client
	else{
		if((sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr*)&client_addr, len) == -1)){
			printf("sendto ERROR!\n");
			exit(1);
		}
	}

	close(sockfd);
	return 0;
}
		
		




