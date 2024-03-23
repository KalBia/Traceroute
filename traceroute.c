/* Kalina Białek 340152 */

#include "receive.h"
#include "send.h"

/* ---------------------------------------------------------------- */

int traceroute(char *IP_addr) {

    /* Create raw socket. */
    /* AF_INET -  the socket will be used for IPv4 addresses.
     * SOCK_RAW - the socket will be a raw socket.
     * IPPROTO_ICMP - the socket will be used for ICMP (Internet Control Message Protocol) packets. */
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

    /* We need pid to make identifier of packages - but we just need 16 digits :D */
    uint16_t pid = (uint16_t)getpid();

    bool dest_reached = false;
    /* For all TTL in [1, 30] or untill we reach the destination router. */
    for(int TTL = 1; TTL <= 30; TTL++) {

        /* Send three packets to the destination router at the same time. */
        send_packets(sockfd, TTL, pid, IP_addr);

        /* Wait for the responses. */
        char *result = malloc(100*sizeof(char));
        int ret = wait_for_packets(sockfd, pid, (uint16_t)TTL, result, &dest_reached, IP_addr);
        if (ret) {
            fprintf(stderr, "wait_for_packets error: %s\n", strerror(errno));
		    return EXIT_FAILURE;
        }

        /* Manage the "TTL test". */
        /* Print the result. */
        char* space = "";
        if(TTL < 10)
            space = " ";

        printf("%d.%s %s\n", TTL, space, result);

        if(dest_reached) {
            free(result);
            break;
        }

    }

    /* Cleaning up and stuff. */

    if(close(sockfd)) {
        fprintf(stderr, "closing socket error: %s\n", strerror(errno));
		return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------- */

int main(int argc, char *argv[]) {

    /* Check if argument was given. */
    if(argc != 2) {
        fprintf(stderr, "wrong number of arguments\n");
		return EXIT_FAILURE;
    }

    /* We need to be "sudo". */
    if(geteuid()) {
        fprintf(stderr, "you must be root (rude?)\n");
		return EXIT_FAILURE;
    }

    if(traceroute(argv[1])) {
        fprintf(stderr, "traceroute error\n");
		return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
