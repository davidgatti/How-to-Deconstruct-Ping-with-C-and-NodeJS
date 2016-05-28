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
