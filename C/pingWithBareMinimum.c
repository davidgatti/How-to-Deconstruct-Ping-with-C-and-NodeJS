#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

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
        uint32_t data;
    } icmp_hdr_t;

    //
    //  3. Use the newly created struct to make a variable.
    //
    icmp_hdr_t pckt;

    //
    //  4. Set the appropriate values to our struct, which is our ICMP header
    //
    pckt.type = 8;          // The echo request is 8
    pckt.code = 0;          // No need
    pckt.chksum = 0xfff7;   // Fixed checksum since the data is not changing
    pckt.data = 0;          // We don't send anything.

    //
    //  5. Creating a IP Header from a struct that exists in another library
    //
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    //
    //  6. Send our PING
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
    // 7. Prepare all the necessary variable to handle the response
    //
    unsigned int resAddressSize;
    unsigned char res[30] = "";
    struct sockaddr resAddress;

    //
    //  8. Read the response from the remote host
    //
    int ressponse = recvfrom(s, res, sizeof(res), 0, &resAddress,
                             &resAddressSize);

    //
    //  -> Display the response in its raw form (hex)
    //
    if( ressponse > 0)
    {
        printf("Message is %d bytes long, and looks like this:\n", ressponse);

        for(int i = 0; i < ressponse; i++)
        {
            printf("%x ", res[i]);
        }

        printf("\n");

        exit(0);
    }
    else
    {
        perror("Response Error");
        exit(0);
    }

    return 0;
}
