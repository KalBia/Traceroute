/* Kalina Białek 340152 */

#ifndef SEND_H
#define SEND_H

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

int send_packets(int sockfd, int TTL, uint16_t pid, char* IP_addr);

#endif
