#include "icmp_tools.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

static uint16_t compute_icmp_checksum(const void *buff, int length) {
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
        handle_error("setsockopt");
    }

    ssize_t bytes_sent = sendto(sockfd, &icmp_header, sizeof(icmp_header), 0,
                               (struct sockaddr *)&recipient, sizeof(recipient));

    if (bytes_sent < 0) {
        handle_error("sendto");
    }
}

static int time_passed(int packets_received, struct timeval *current_time,
                       struct timeval *end_time, int nqueries) {
    if (packets_received >= nqueries || timercmp(current_time, end_time, >)) {
        return 1;
    }

    return 0;
}

/*
    returns 1 when host is reached
        and 0 otherwise
*/
int wait_for_icmps(int sockfd, uint16_t pid, uint8_t ttl, struct timeval *start_time,
                    struct timeval *end_time, int nqueries) {
    int packets_received = 0;
    int host_reached = 0;

    struct timeval deltas[nqueries];
    char ips[nqueries][20];
    struct timeval current_time;

    printf("%d. ", ttl);

    gettimeofday(&current_time, NULL);
    while (!time_passed(packets_received, &current_time, end_time, nqueries)) {
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);
        uint8_t buffer[IP_MAXPACKET];

        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(sockfd, &descriptors);
        struct timeval tv;
        timersub(end_time, &current_time, &tv);
        int ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);
        if (ready < 0) {
            handle_error("select");
        } if (ready == 0) {
            break;
        }

        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);
        if (packet_len < 0) {
            handle_error("recvfrom");
        }

        gettimeofday(&current_time, NULL);

        char sender_ip_str[20];
        const char *inet_ntop_ret = inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
        assert(inet_ntop_ret != NULL);

        struct iphdr *ip_header = (struct iphdr *) buffer;
        ssize_t ip_header_len = 4 * ip_header->ihl;

        struct icmphdr *icmp_ptr = (struct icmphdr *)(buffer + ip_header_len);

        uint8_t icmp_type = icmp_ptr->type;
        int proper_type = icmp_type == ICMP_TIME_EXCEEDED || icmp_type == ICMP_ECHOREPLY;

        if (icmp_type == ICMP_TIME_EXCEEDED) {
            struct iphdr *inner_ip_header = (void *) icmp_ptr + 8;
            ssize_t inner_ip_header_len = 4 * inner_ip_header->ihl;
            icmp_ptr = (void *)inner_ip_header + inner_ip_header_len;
        }

        if (proper_type && icmp_ptr->un.echo.id == pid && icmp_ptr->un.echo.sequence == ttl) {
            timersub(&current_time, start_time, &deltas[packets_received]);

            int new_ip = 1;
            for (int i = 0; i < nqueries; i++) {
                if (memcmp(ips[i], sender_ip_str, 20) == 0) {
                    new_ip = 0;
                }
            }

            if (new_ip) {
                printf("%s ", sender_ip_str);
                memcpy(ips[packets_received], sender_ip_str, 20);
            }

            packets_received++;
            if (icmp_type == ICMP_ECHOREPLY) {
                host_reached = 1;
            }
        }
    }

    if (packets_received == 0) {
        printf("*");
    } else if (packets_received < nqueries) {
        printf("???");
    } else {
        double average_time = 0;
        for (int i = 0; i < packets_received; i++) {
            average_time += deltas[i].tv_usec/1000.0;
        }
        average_time /= nqueries;

        printf("%.1f ms", average_time);
    }

    printf("\n");
    // for (int i = 0; i < packets_received; i++) {
    // 	printf("%.1fms ", deltas[i].tv_usec/1000.0);
    // }
    // printf("\n");

    return host_reached;
}