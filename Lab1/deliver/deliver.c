//most of the codes are from 'Beej's guide' and 'GeeksforGeeks'
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define MAXLINE 100
#define MAX 200


int main(int argc, char *argv[]){

	//checking Execution Command
	if(argc != 3){
		printf("Invalid number of arguments: %s \n", argc);
		printf("Usage: deliver <server address> <server port number>\n");
		exit(1);
	}
	
	if(strcmp(argv[0], "deliver") != 0){
		printf("Invalid command: %s \n", argv[0]);
		printf("Usage: deliver <server address> <server port number>\n");
		exit(1);
	}

	int n;
	int sendtoVal = 0;
	int port = atoi(argv[2]);
	printf("port: %d\n", port);

	//creating Socket
	int sockFd;
	struct sockaddr_in server_addr;
	socklen_t len;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

	if((sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		printf("Socket ERROR!\n");
		exit(1);
	}
	printf("sockfd: %d\n", sockFd);

	char buffer[MAXLINE] = {0};

	

	//getting User Input
	char protocol[MAXLINE];
       	char filename[MAXLINE] = {0};
	printf("Follow format to transfer file: ftp <filename> \n");
	scanf("%s %s", protocol, filename);

	if(strcmp(protocol, "ftp") != 0){
		printf("Invalid command: %s \n", protocol);
		exit(1);
	}

	int debug = 0;
	if(access(filename, F_OK) != 0){
		debug = access(filename,F_OK);
		//printf("%d\n", debug);
		printf("File cannot be found \n");
		exit(1);
	}

	
	//dummy send
	//sendto(sockFd, "hello", strlen("hello"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	//send
	if((sendtoVal = sendto(sockFd, "ftp", strlen("ftp"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1){
		printf("sendto ERROR!\n");
		exit(1);
	}
	printf("sendtoVal: %d\n", sendtoVal); //checkpoint
	

	//receive
	n = recvfrom(sockFd, buffer, MAXLINE, 0, (struct sockaddr*)&server_addr, &len);
	buffer[n] = '\0';
	printf("recvfrom\n");

	if(strcmp(buffer, "yes") == 0){
		printf("A file transfer can start.\n");
	}
	else{
		exit(1);
	}

	//Close Socket File Descriptor
	close(sockFd);

	return 0;
}




































