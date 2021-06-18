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
#include <time.h>

#include "packet.h"

#define MAXLINE 100
#define MAX 200
#define DATA_SIZE 1000

char** create_packets(char* filename, Packet* packet){
    FILE* file_pointer;

    if((file_pointer = fopen(filename, "r")) == NULL){ //If file doesn't exist
        printf("File %s doesn't exist \n", filename);
        exit(1);
    }
    
    //For verification
    int total_size = 0;

    //Calculate total_frag
    fseek(file_pointer, 0, SEEK_END); //set to last position
    int total_frag = (ftell(file_pointer)/1000) + 1; //Last position/1000 because 0-999 = 1 packet
    rewind(file_pointer); //Set position to beginning

    //checkpoint
    //printf("Total_frag: %d\n", total_frag);
    
    // Stores packets in packet array
    char **packets = malloc(sizeof(char*) * total_frag);

     for(int packet_number = 1; packet_number < (total_frag + 1); packet_number ++){

        memset(packet->filedata, 0, sizeof(char) * (DATA_SIZE));
        fread((void*)packet->filedata, sizeof(char), DATA_SIZE, file_pointer);

        packet->total_frag = total_frag;

        packet->frag_no = packet_number;

        if(packet_number == total_frag){ //Last Packet size
            fseek(file_pointer, 0, SEEK_END);
            packet->size = (ftell(file_pointer)%1000) + 1;
        }else{
            packet->size = DATA_SIZE;
        }

        //For verification
        total_size += packet->size;

        packet->filename = filename;

        // Save packet to packets array
        packets[packet_number - 1] = malloc(BUF_SIZE * sizeof(char));
        packet2string(packets[packet_number - 1], packet);
        //printf("Saved Packets and Converted to String\n");
        
    }

    printf("Total file size: %d \n", total_size);
    printf("Total number of packets: %d \n", total_frag);

    return packets;
}


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

    int sendtoVal = 0;
    int receivefromVal = 0;
    int port = atoi(argv[2]);
    //printf("port: %d\n", port);

    //creating Socket
    int sockFd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_size = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    if((sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        printf("Socket ERROR!\n");
        exit(1);
    }
    //printf("sockfd: %d\n", sockFd);

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


    //check "ftp"
    //send
    if((sendtoVal = sendto(sockFd, "ftp", strlen("ftp"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1){
            printf("sendto ERROR!\n");
            exit(1);
    }
    //printf("sendtoVal: %d\n", sendtoVal); //checkpoint

    //receive
    int n;
    socklen_t len;
    n = recvfrom(sockFd, buffer, MAXLINE, 0, (struct sockaddr*)&server_addr, &len);
    buffer[n] = '\0';
    //printf("recvfrom\n");
    //printf("buffer: %s\n", buffer);

    if(strcmp(buffer, "yes") == 0){
            printf("A file transfer can start.\n");
    }
    else{
            exit(1);
    }
    

    

    //Create Packets
    Packet packet;
    char **packets = create_packets(filename, &packet);
    // CHECKPOINT

    //Open File
    FILE* file_pointer;

    if((file_pointer = fopen(filename, "r")) == NULL){ //If file doesn't exist
        printf("File %s doesn't exist \n", filename);
        exit(1);
    }

    //Calculate total_frag
    fseek(file_pointer, 0, SEEK_END); //set to last position
    int total_frag = (ftell(file_pointer)/1000) + 1; //Last position/1000 because 0-999 = 1 packet
    rewind(file_pointer); //Set position to beginning

    //For verification
    int total_size = 0;
    
    //Creating ACK buffer
    Packet ACK_packet; //to save values from server
    char ACK_buf[BUF_SIZE];
    memset(ACK_buf, 0, sizeof(char)* BUF_SIZE);


    // Sending and Waiting for ACK
    for(int packet_number = 1; packet_number < (packet.total_frag + 1); packet_number ++){
        //Start timer
        clock_t start_time, end_time;
        start_time = clock();

        // Send 1 Packet
        //printf("Packet Info Before Sending: %s\n", packets[packet_number - 1]);

        if((sendtoVal = sendto(sockFd, packets[packet_number - 1], BUF_SIZE, 0, (struct sockaddr*)&server_addr, server_addr_size)) == -1){
            printf("sendto ERROR: packet #%d \n", packet_number);
            exit(1);
        }else{
            printf("Sent Packet: %d \n", packet_number); //checkpoint
        }

        // Receive 1 ACK
        if((receivefromVal = recvfrom(sockFd, ACK_buf, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_size)) == -1){
            printf("receivefrom ERROR: ACK for packet #%d \n", packet_number);
            exit(1);
        }

        //End timer
        end_time = clock();

        //Print Round Trip time
        double RT_time = end_time - start_time;
        printf("RTT: %f\n", (RT_time / CLOCKS_PER_SEC));
        
        //Checkpoint
        //printf("Packet Info: %s\n", ACK_buf);

        string2packet(ACK_buf, &ACK_packet);

        //Checkpoint
        //printf("Frag_no: %d\n", ACK_packet.frag_no);
        //printf("Total_frag: %d\n", ACK_packet.total_frag);
        //printf("Size: %d\n", ACK_packet.size);
        //printf("filename: %s\n", ACK_packet.filename);
        //printf("Filedata: %s\n", ACK_packet.filedata);

        if((strcmp(ACK_packet.filename, filename) == 0) 
        && (ACK_packet.frag_no == packet_number) 
        && (strcmp(ACK_packet.filedata, "ACK") == 0)){
            printf("Received ACK for Packet: %d \n", packet_number);
        }else{
            printf("Not Received ACK for Packet: %d \n", packet_number);
            exit(1);
        }
    }


    //Close Socket File Descriptor
    close(sockFd);

    return 0;
}