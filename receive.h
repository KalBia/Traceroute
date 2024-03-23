/* Kalina Białek 340152 */

#ifndef RECEIVE_H
#define RECEIVE_H

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

int wait_for_packets(int sockfd, uint16_t pid, uint16_t seq, char* result, bool* dest_reached, char* dest_router);

#endif
