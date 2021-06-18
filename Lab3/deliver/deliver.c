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
#include <stdbool.h>
#include <math.h>

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

    //measuring initial RTT
    clock_t start_time, end_time;
    start_time = clock();
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

    end_time = clock();
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

    //initializing variables to be used..
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int sentNum = 0; //number of trials
    int packet_number = 1;

    clock_t InitialRTT = end_time - start_time;
    clock_t EstimatedRTT = InitialRTT * 2;
    clock_t DevRTT = InitialRTT;
    clock_t SampleRTT;
    clock_t new_start_time;
    clock_t new_end_time;

    bool isTimeOut = false;


    while(packet_number < (packet.total_frag + 1)){

        isTimeOut = false;

        //time measure start
        new_start_time = clock();
        if((sendtoVal = sendto(sockFd, packets[packet_number - 1], BUF_SIZE, 0, (struct sockaddr*)&server_addr, server_addr_size)) == -1){
            printf("sendto ERROR: packet #%d \n", packet_number);
            exit(1);
        }

        //loop to get ACK
        while(1){

            if((receivefromVal = recvfrom(sockFd, ACK_buf, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_size)) == -1){
                sentNum++;
                printf("receivefrom ERROR: ACK for packet #%d , Attempt #%d to resend\n", packet_number, sentNum);

                if(sentNum < 5){
                    isTimeOut = true; //resend (limit not reached)
                    break; //leaving the closest while loop
                }
                else{
                    printf("Resend limit reached. Terminating file transfer.. :(\n");
                    exit(1);
                }
            }

            //converting..
            string2packet(ACK_buf, &ACK_packet);

            //checking if ack number corresponds to sequence number
            if((strcmp(ACK_packet.filename, filename) == 0) && (ACK_packet.frag_no == packet_number) && (strcmp(ACK_packet.filedata, "ACK") == 0)){
                printf("Received ACK for Packet: %d \n", packet_number);
                break; //leaving the closest while loop (ack matches; send next packet)
            }
            else{
                printf("ACK does not match sent packet. Dropping packet.. :(\n");
            }
        }//end of inner while loop

        //time measure end
        new_end_time = clock();

        //timeout interval calculation
        SampleRTT = new_end_time - new_start_time;
        EstimatedRTT = 0.875 * ((double) EstimatedRTT) + 0.125 * ((double) SampleRTT);
        DevRTT = 0.75 * ((double) DevRTT) + 0.25 * fabs((double)SampleRTT - (double)EstimatedRTT);
        timeout.tv_usec = (double)EstimatedRTT + 4 * (double)DevRTT;
        //testing timeout functionality
        //timeout.tv_sec = 5;
        //timeout.tv_usec = 999999;

        if(setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == -1){
            printf("setsockopt ERROR2!\n");
        }

        //checking if resend is needed
        if(isTimeOut == true){
            printf("Resending packet #%d\n", packet_number);
        }
        else{
            sentNum = 0; //reset 
            packet_number++; //send next packet
        }
    }//end of outer while loop

    //FIN packet send
    Packet fin_packet;
    char FIN_buf[BUF_SIZE];
    memset(FIN_buf, 0, sizeof(char)* BUF_SIZE);

    fin_packet.total_frag = total_frag;
    fin_packet.frag_no = 0;
    fin_packet.size = DATA_SIZE;
    fin_packet.filename = filename;
    strcpy(fin_packet.filedata, "FIN");

    packet2string(FIN_buf, &fin_packet);

    if((sendtoVal = sendto(sockFd, FIN_buf, BUF_SIZE, 0, (struct sockaddr*)&server_addr, server_addr_size)) == -1){
        printf("sendto ERROR: FIN packet\n");
        exit(1);
    }


    //Close Socket File Descriptor
    close(sockFd);

    return 0;
}