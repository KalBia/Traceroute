/* Kalina Białek 340152 */

#include "send.h"

/* ---------------------------------------------------------------- */

u_int16_t compute_icmp_checksum (const void *buff, int length)
{
    const u_int16_t* ptr = buff;
    u_int32_t sum = 0;
    assert (length % 2 == 0);
    for (; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16U) + (sum & 0xffffU);
    return (u_int16_t)(~(sum + (sum >> 16U)));
}

/* ---------------------------------------------------------------- */

int send_packets(int sockfd, int TTL, uint16_t pid, char* IP_addr) {

    /* Construct echo request message. */
    struct icmp header;
    bzero(&header, sizeof(header));
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_id = htons(pid);

    /* Add destination IP address. */
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    if (inet_pton(AF_INET, IP_addr, &recipient.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error in send: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* Change TTL in IP header. */
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &TTL, sizeof(int));

    /* Send packets. */
    for(int i = 0; i < 3; i++) {
        /* We want to give different sequence to every packet.
         * Actually, I had packets with the same sequence and it worked the same... */
        header.icmp_seq = htons(3*(uint16_t)TTL + i);
        header.icmp_cksum = 0;
        header.icmp_cksum = compute_icmp_checksum((u_int16_t*)&header, sizeof(header));

        ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
        if(bytes_sent < 0) {
            fprintf(stderr, "sendto error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
