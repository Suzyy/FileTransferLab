#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <regex.h>

#define BUF_SIZE 1300
#define DATA_SIZE 1000

typedef struct Packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[DATA_SIZE];
} Packet;

// Packet --> String --> Buffer
// Packet --> "int:int:int:str:str"
// ex) "3:2:10:footbar.txt:lo World!\n"
void packet2string(char *buf, Packet *packet){
    memset(buf, 0, BUF_SIZE);

    int loc = 0;

    sprintf(buf, "%d", packet -> total_frag); // buf = [3]
    loc = strlen(buf);                     // loc = 1
    memcpy(buf + loc, ":", sizeof(char));  // buf = [3:]
    ++ loc;                                // loc = 2

    sprintf(buf + loc, "%d", packet -> frag_no); // buf = [3:2]
    loc = strlen(buf);                           // loc = 3
    memcpy(buf + loc, ":", sizeof(char));        // buf = [3:2:]
    ++ loc;                                      // loc = 4

    sprintf(buf  + loc, "%d", packet -> size);   // buf = [3:2:10]
    loc = strlen(buf);                           // loc = 6
    memcpy(buf + loc, ":", sizeof(char));        // buf = "3:2:10:"
    ++ loc;                                      // loc = 11
    
    sprintf(buf  + loc, "%s", packet -> filename);
    loc += strlen(packet->filename);
    memcpy(buf + loc, ":", sizeof(char));
    ++ loc;
    
    memcpy(buf + loc, packet->filedata, sizeof(char)*DATA_SIZE);
}

// Buffer --> String --> Packet
// "int:int:int:str:str" -> Packet
void string2packet(char *buf, Packet *packet){
    //initializing variables to be used
    unsigned int total_frag, frag_no, size;
    char* filename;
    char filedata[1000];
    //strings need to be converted to int
    char* total_frag_s;
    char* frag_no_s;
    char* size_s; 
    int header_loc; 
    
    //splitting input string into smaller strings
    total_frag_s = strtok(buf, ":");
    frag_no_s = strtok(NULL, ":");
    size_s = strtok(NULL, ":");
    filename = strtok(NULL, ":");
    
    //getting the location of the header (where filedata begins)
    header_loc = strlen(total_frag_s) + strlen(":") + strlen(frag_no_s) + strlen(":") + strlen(size_s) + strlen(":") + strlen(filename) + strlen(":");
    
    //converting string to integer
    total_frag = atoi(total_frag_s);
    frag_no = atoi(frag_no_s);
    size = atoi(size_s);
    
    //printf("Header_loc: %d\n", header_loc);

    //obtaining filedata from buffer
    memcpy(filedata, buf + header_loc, size);
    
    //assigning obtained variables to packet structure
    packet->total_frag = total_frag;
    packet->frag_no = frag_no;
    packet->size = size;
    packet->filename = filename;
    memcpy(packet->filedata, filedata, size);
}