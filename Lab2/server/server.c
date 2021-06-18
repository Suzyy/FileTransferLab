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

#include "packet.h"

#define BUF_SIZE 1300

int main(int argc, char **argv){
    if(argc != 2){ //bad argument
        printf("Usage: server <UDP listen port>\n");
    }
    //Declaring variables used in this program
    int port = atoi(argv[1]); //assigning port
    //printf("port: %d\n", port);
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
    //printf("sockfd: %d\n", sockfd); //checkpoint 1

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

    len = sizeof(client_addr); //length of client address

    //Receiving message from the client
    //n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len);
    //printf("checkpoint\n");
    //buf[n] = '\0';

    //dummy recieve
    //recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len);
    //printf("recieved message: %s\n", buf);

    n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len);
    if(n == -1){
        printf("recvfrom ERROR!\n");
        exit(1);
    }
    buf[n] = '\0';
    //printf("recvfrom done\n");

    //Sending message back to client
    //if the message is "ftp", reply with a message "yes" to the client
    //printf("buffer: %s\n", buf);
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
    

    //Recieve packet from client (ftp section3)
    //Using packet structure
    Packet packet; 
    char *filename;
    char buf2[BUF_SIZE]; //buffer used to recieve string from client
	char buf3[BUF_SIZE];
    FILE *fp = NULL;
    int loop = 1;

    while(loop){
        //recieve string from client
        if(recvfrom(sockfd, buf2, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &len) == -1){
            printf("recvfrom ERROR!\n");
            exit(1);
        }
        
        //CHECKPOINT
        //printf("Received\n");

        //convert string into packet
        string2packet(buf2, &packet);
        printf("Received Packet: %d\n",packet.frag_no);

        //CHECKPOINT
        //printf("Converted\n");

        //printf("Frag_no: %d\n", packet.frag_no);
        //printf("Total_frag: %d\n", packet.total_frag);
        //printf("Size: %d\n", packet.size);
        //printf("filename: %s\n", packet.filename);
        //printf("Filedata: %s\n", packet.filedata);

        //if recieved first fragment, assign filename and open to write.
        if(packet.frag_no == 1){
            //CHECKPOINT
            //printf("Entered if\n");

            //strcpy(filename, packet.filename);
            //printf("filename: %s\n", filename);

            fp = fopen(packet.filename, "w");

            //CHECKPOINT
            //printf("Received 1st packet\n");
        }

        //write filedata into the local file
        if(fwrite(packet.filedata, sizeof(char), packet.size, fp) != packet.size){
            printf("fwrite ERROR!\n");
            exit(1);
        }

        //write acknowledgement to guarantee correct receipt of the file
        //strcpy function simply overwrites the original string
        char* ACK = "ACK";
        strcpy(packet.filedata, ACK);

        //convert modified packet into string
        packet2string(buf3, &packet);

        //send string to client
        //printf("Packet Info Before Sending: %s \n", buf3);

        if((sendto(sockfd, buf3, BUF_SIZE, 0, (struct sockaddr*)&client_addr, len) == -1)){
                printf("sendto ERROR!\n");
                exit(1);
        }

        printf("Sent ACK for Packet: %d\n", packet.frag_no);

        //if recieved last fragment, end loop : DONE
        if(packet.frag_no == packet.total_frag){
            printf("Congratulations! File transfer success! YAYY!!\n");
            loop = 0;
        }
    }

    close(sockfd);
    return 0;
}