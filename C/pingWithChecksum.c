#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

int32_t checksum(uint16_t *buf, int32_t len);

int main() {

    //sudo make && sudo ./ping.out && make clean

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
        uint16_t identifier;
        uint16_t sequence_number;
        uint32_t data;
    } icmp_hdr_t;

    icmp_hdr_t pckt;

    pckt.type = 8;
    pckt.code = 0;
    pckt.identifier = getpid();
    pckt.sequence_number = 1;
    pckt.chksum = 0;
    pckt.data = 0;

    // Calculate the checksum based on the whole header, where the initial
    // value for the checksum field must be set to 0
    pckt.chksum = checksum((uint16_t *)&pckt, sizeof(pckt));

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
    unsigned char res[100] = "";
    struct sockaddr resAddress;

    // The struct for the ICMP Echo reply
    typedef struct {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequence_number;
    } icmp_response_t;

    // Read the response
    int ressponse = recvfrom(s, res, sizeof(res), 0, &resAddress, &resAddressSize);

    // If we have bytes then lets display them
    if(ressponse > 0) {

        printf("Response is %d bytes long, and has the following content:\n", ressponse);

        // Map our resposne to our response struct
        icmp_response_t* echo_response;
        echo_response = (icmp_response_t *)&res[20];

        // Log the data that we'v got back
        printf(
            "type: %x, code: %x, checksum: %x, identifier: %d, sequence: %d\n",
            echo_response->type,
            echo_response->code,
            echo_response->checksum,
            echo_response->identifier,
            echo_response->sequence_number
        );

        exit(0);

    } else {

        perror("Response Error");
        exit(0);

    }

    return 0;
}

int32_t checksum(uint16_t *buf, int32_t len)
{
    int32_t nleft = len;
    int32_t sum = 0;
    uint16_t *w = buf;
    uint16_t answer = 0;

    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if(nleft == 1)
    {
        *(uint16_t *)(&answer) = *(uint8_t *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}
