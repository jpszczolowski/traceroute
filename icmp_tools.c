#include "icmp_tools.h"

#include <stdio.h> // ???
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

uint16_t compute_icmp_checksum(const void *buff, int length) {
	uint32_t sum;
	const uint16_t *ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (uint16_t)(~(sum + (sum >> 16)));
}

void send_single_icmp(int sockfd, const char *ip, uint16_t id, uint16_t sequence, int ttl) {
	struct icmphdr icmp_header;
	icmp_header.type = ICMP_ECHO;
	icmp_header.code = 0;
	icmp_header.un.echo.id = id;
	icmp_header.un.echo.sequence = sequence;
	icmp_header.checksum = 0;
	icmp_header.checksum = compute_icmp_checksum((uint16_t *)&icmp_header, sizeof(icmp_header));

	struct sockaddr_in recipient;
	bzero(&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
	int inet_pton_ret = inet_pton(AF_INET, ip, &recipient.sin_addr);
	assert(inet_pton_ret == 1);

	int setsockopt_ret = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
	if (setsockopt_ret != 0) {
		handle_error("setsockopt error");
	}

	ssize_t bytes_sent = sendto(sockfd, &icmp_header, sizeof(icmp_header), 0,
							   (struct sockaddr *)&recipient, sizeof(recipient));

	if (bytes_sent < 0) {
		handle_error("sendto error");
	}
}

void wait_one_sec_for_icmps(int sockfd, u_int16_t pid, u_int8_t ttl, struct timeval *start_time) {
	struct sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);
	u_int8_t buffer[IP_MAXPACKET];

	ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);
	if (packet_len < 0) {
		handle_error("recvfrom error");
	}

	char sender_ip_str[20];
	const char *inet_ntop_ret = inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
	assert(inet_ntop_ret != NULL);

	printf("Received IP packet with ICMP content from: %s\n", sender_ip_str);

	struct iphdr *ip_header = (struct iphdr *) buffer;
	ssize_t ip_header_len = 4 * ip_header->ihl;

	struct icmphdr *icmp_ptr = (struct icmphdr *)(buffer + ip_header_len);

	if (icmp_ptr->type == ICMP_TIME_EXCEEDED) {
		printf("TTL exceeded\n");
		struct iphdr *inner_ip_header = (void *) icmp_ptr + 8;
		ssize_t inner_ip_header_len = 4 * inner_ip_header->ihl;
		icmp_ptr = (void *)inner_ip_header + inner_ip_header_len;
	} else if (icmp_ptr->type == ICMP_ECHOREPLY) {
		printf("reply from server\n");
	} else {
		printf("reply with code %d\n", icmp_ptr->type);
	}

	printf("pid %hd, ttl %hd\n\n", icmp_ptr->un.echo.id, icmp_ptr->un.echo.sequence);
}