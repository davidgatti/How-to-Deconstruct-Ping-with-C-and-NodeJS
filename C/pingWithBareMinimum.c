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

    if(s > 0) {

        printf("Socket created\n");

	} else {

        perror("Socket Error");
        exit(0);

    }

    //
    // 2. Settign options
    //
    const int ttl = 30;

    int actionSocketOptions = setsockopt(s, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    if(actionSocketOptions >= 0) {

        printf("Options set\n");

    } else {

        perror("Options Error");
        exit(0);

    }

    //
    // 3. Sending PING
    //

    // ICMP Header
    typedef struct {
        uint8_t type;
        uint8_t code;
        uint16_t chksum;
        uint32_t data;
    } icmp_hdr_t;

    icmp_hdr_t pckt;

    pckt.type = 8;
    pckt.code = 0;
    pckt.chksum = 0xfff7;
    pckt.data = 0;

    // IP Header
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    int actionSendResult = sendto(s, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(actionSendResult > 0) {

        printf("Ping sent at %d bytes\n", actionSendResult);

    } else {

        perror("Ping Error");
        exit(0);

    }

    //
    // 4. Readign the response
    //
    unsigned int resAddressSize;
    unsigned char res[30] = "";
    struct sockaddr resAddress;

    int ressponse = recvfrom(s, res, sizeof(res), 0, &resAddress, &resAddressSize);

    if( ressponse > 0) {

        printf("Message is %d bytes: ", ressponse);

        for(int i = 0; i < ressponse; i++) {

            printf("%x ", res[i]);

        }

        printf("\n");

        exit(0);

    } else {

        perror("Response Error");
        exit(0);

    }

    return 0;
}
