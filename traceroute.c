#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "icmp_tools.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <IPv4 address>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct in_addr converted_ip;
	if (inet_pton(AF_INET, argv[1], &converted_ip) != 1) {
		fprintf(stderr, "IP address not valid!\nUsage: %s <IPv4 address>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		handle_error("socket");
	}

	u_int16_t pid = getpid(); // pid modulo 2^16

	int nqueries = 3;
	for (int ttl = 1; ttl <= 30; ttl++) {
		struct timeval start_time, end_time;
		gettimeofday(&start_time, NULL);
		end_time = start_time;
		end_time.tv_sec++;

		for (int i = 0; i < nqueries; i++) {
			send_single_icmp(sockfd, argv[1], pid, ttl, ttl);
		}

		int host_reached = wait_for_icmps(sockfd, pid, ttl, &start_time, &end_time, nqueries);
		if (host_reached) {
			break;
		}
	}

	return EXIT_SUCCESS;
}
