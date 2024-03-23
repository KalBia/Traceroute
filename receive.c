/* Kalina Białek 340152 */

#include "receive.h"

/* ---------------------------------------------------------------- */

/* Get the pointers to information in packet.
 * [IP header][ICMP header][ICMP data] */
struct icmp* get_icmp_header(u_int8_t* buffer) {

    struct ip* ip_header = (struct ip*)buffer;
    ssize_t	ip_header_len = 4 * (ssize_t)(ip_header->ip_hl);
    u_int8_t* icmp_packet = buffer + ip_header_len;
    return (struct icmp*)icmp_packet;

}

/* ---------------------------------------------------------------- */

bool check_packet(struct icmp* icmp_header, uint16_t pid, uint16_t seq) {

    /* If it is TIME_EXCEEDED message, than we need to move our "header". */
    if(icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
        icmp_header = get_icmp_header( (u_int8_t*)icmp_header + (ssize_t)sizeof(struct icmphdr) );
    }
    else if(icmp_header->icmp_type != ICMP_ECHOREPLY) /* not the type we were looking for :c */
        return false;

    /* Now we have icmp_header pointing to right place in both TIME_EXCEEDED and ECHOREPLY :D
     * Check if the id and seq are correct. */
    if(icmp_header->icmp_id == htons(pid) && (ntohs(icmp_header->icmp_seq))/3 == seq)
        return true;
    return false;
}

/* ---------------------------------------------------------------- */

/* Manages everything around single packet received. */
int manage_packet(int sockfd, uint16_t pid, uint16_t seq, char ip_addresses[3][20], int index_ip_addresses) {

    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];

    ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
    if (packet_len < 0) {
        fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
        return -1;
    }

    /* Get the pointers to information in packet. */
    struct icmp* icmp_header = get_icmp_header(buffer);

    /* Check if it's "our" packet. */
    if(!check_packet(icmp_header, pid, seq)) /* packet wasn't the one we were waiting for :c */ {
        return 0;
    }

    /* Changes address of sender to string. */
    char sender_ip_str[20];
    if (!inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str))) {
        fprintf(stderr, "inet_ntop error in receive: %s\n", strerror(errno));
        return -1;
    }
    strcpy(ip_addresses[index_ip_addresses], sender_ip_str);

    return 1;
}

/* ---------------------------------------------------------------- */

/* Returns:
 *  in result - string that will be displayed
 *  bool - if we reached the destination router
 * */
bool make_result(char* result, double responses[3], char ip_addresses[3][20], int nr_packets, char* dest_router) {

    /* strcp if false - first time writing to string, strcat if we want to add another string. */
    bool cp_or_cat = false;
    bool dest_reached = false;

    /* Prepare string with IP addresses */
    for(int i = 0; i < nr_packets; i++) {
        bool already_added = false;

        /* Check if destination was reached. */
        if(!strcmp(ip_addresses[i], dest_router))
            dest_reached = true;

        /* If it is the same address as before - we don't want to add it. */
        for(int ii = i-1; ii >= 0; ii--) {
            if(!strcmp(ip_addresses[ii], ip_addresses[i]))
                already_added = true;
        }

        if(!already_added) {
            if(!cp_or_cat)
                strcpy(result, ip_addresses[i]);
            else {
                strcat(result, " ");
                strcat(result, ip_addresses[i]);
            }
        }
    }

    /* Prepare part of the result with * or ??? or average time. */
    if(nr_packets == 0) /* No response at all. */ {
        strcpy(result, "*");
    }
    else if(nr_packets == 3) /* We got all three responses - we can calculate average time. */ {
        double sum = 0;
        for(int i = 0; i < 3; i++)
            sum += responses[i];
        sum /= 3;

        char temp[100];
        snprintf(temp, sizeof(temp), "%.3f", sum);

        strcat(result, "   ");
        strcat(result, temp);
        strcat(result, "ms");
    }
    else /* So there were responses, but not three :c */ {
        strcat(result, "   ???");
    }

    return dest_reached;
}

/* ---------------------------------------------------------------- */

/* Returns the result of current TTL test in char* result and in bool if we reached the destination. */
int wait_for_packets(int sockfd, uint16_t pid, uint16_t seq, char* result, bool* dest_reached, char* dest_router) {

    /* Why not poll()? It does not update timeout!
     * "On Linux, select() modifies timeout to reflect the amount of time
      * not slept; most other implementations do not do this." */

    /* Set up fd_set structure. */
    fd_set ps;
    FD_ZERO(&ps);
    FD_SET(sockfd, &ps);

    /* We wait for 3 packets or 1 second. */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int nr_packets = 0;

    /* We need to build result later. */
    char ip_addresses[3][20];
    double responses[3];
    int index_responses = 0;
    int index_ip_addresses = 0;

    /* Wait for packets. */
    while (nr_packets < 3) {
        int ready = select(sockfd+1, &ps, NULL, NULL, &timeout); /* wait 1 second */

        if(ready < 0) {
            fprintf(stderr, "select() error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        else if (ready == 0) /* timeout! */ {
            /* We move on to the next TTL test. */
            break;
        }

        int ret = manage_packet(sockfd, pid, seq, ip_addresses, index_ip_addresses);
        if(ret < 0) {
            fprintf(stderr, "manage_packets error\n");
            return EXIT_FAILURE;
        }

        if(ret != 0) {
            nr_packets++;
            index_ip_addresses++;
            responses[index_responses++] = ((double)(1000000 - timeout.tv_usec))/1000;
        }
    }

    /* Return result. */
    *dest_reached = make_result(result, responses, ip_addresses, nr_packets, dest_router);
    return EXIT_SUCCESS;
}
