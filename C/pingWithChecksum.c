#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

int32_t checksum(uint16_t *buf, int32_t len);

int main() {

    //
    // 1. Creating Socket
    //
    int s = socket(PF_INET, SOCK_RAW, 1);

    //
    //  -> Exit the app if the socket failed to be created
    //
    if(s <= 0)
    {
        perror("Socket Error");
        exit(0);
    }

    //
    // 2. Create the ICMP Struct Header
    //
    typedef struct {
        uint8_t type;
        uint8_t code;
        uint16_t chksum;
        uint16_t identifier;
        uint16_t sequence_number;
        uint32_t data;
    } icmp_hdr_t;

    //
    //  3. Use the newly created struct to make a variable.
    //
    icmp_hdr_t pckt;

    //
    //  4. Set the apropriate values to our struct, which is our ICMP header
    //
    pckt.type = 8;              // The echo request is 8
    pckt.code = 0;              // No need
    pckt.chksum = 0;            // The chacksum first needs to be calcualted
    pckt.identifier = getpid(); // Set a random Nr. in this case the app ID
    pckt.sequence_number = 1;   // Normaly you would increment this nummber
    pckt.data = 0;              // We don't send anything.

    // 
    //  5. Calculate the checksum based on the whole header, and only then
    //     you add it to the header.
    // 
    pckt.chksum = checksum((uint16_t *)&pckt, sizeof(pckt));

    //
    //  6. Creatign a IP Header from a struct that exists in another library
    //  
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    //
    //  7. Send our PING 
    //
    int actionSendResult = sendto(s, &pckt, sizeof(pckt), 
                                  0, (struct sockaddr*)&addr, sizeof(addr));

    //
    //  -> Exit the app if the option failed to be set
    //
    if(actionSendResult < 0)
    {
        perror("Ping Error");
        exit(0);
    }

    //
    //  8. Prepare all the necesary variable to handle the response
    //
    unsigned int resAddressSize;
    unsigned char res[30] = "";
    struct sockaddr resAddress;

    // 
    //  9. Creating the struct to better handle the response
    // 
    typedef struct {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequence_number;
    } icmp_response_t;

    //
    //  10. Read the response from the remote host
    //
    int ressponse = recvfrom(s, res, sizeof(res), 0, &resAddress, 
                             &resAddressSize);

    //
    //  -> Display the response by accessign the struct
    //
    if(ressponse > 0)
    {
        // 
        //  11. Create the response variable usign our custom struct
        // 
        icmp_response_t* echo_response;

        //
        //  12. Map our resposne to our response struct starting from byte 20
        //
        echo_response = (icmp_response_t *)&res[20];

        // 
        //  -> Log the data that we'v got back
        // 
        printf(
            "type: %x, code: %x, checksum: %x, identifier: %x, sequence: %x\n",
            echo_response->type,
            echo_response->code,
            echo_response->checksum,
            echo_response->identifier,
            echo_response->sequence_number
        );

        exit(0);
    } 
    else 
    {
        perror("Response Error");
        exit(0);
    }

    return 0;
}

//
//  () Checksum Method
//
int32_t checksum(uint16_t *buf, int32_t len)
{
    //
    //  1. Variable needed for the calculation
    //
    int32_t nleft = len;    // Save how big is the header
    int32_t sum = 0;        // Container for the calcualted value
    uint16_t *w = buf;      // Save the first 2 bytes of the header
    uint16_t answer = 0;    // The state of our final anwser

    //
    //  2. Summ evry other byte from the header
    //
    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    //
    //  3. No idea
    //
    if(nleft == 1)
    {
        *(uint16_t *)(&answer) = *(uint8_t *)w;
        sum += answer;
    }

    //
    //  4. Needed conversions
    //
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    //
    //  5. Invert the bits
    //
    answer = ~sum;

    //
    //  -> Result
    //
    return answer;
}
