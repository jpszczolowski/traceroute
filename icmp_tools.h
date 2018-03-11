#include <stdint.h>
#include <sys/time.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); \
						  } while (0)

uint16_t compute_icmp_checksum(const void *buff, int length);
void send_single_icmp(int sockfd, const char *ip, uint16_t id, uint16_t sequence, int ttl);
void wait_one_sec_for_icmps(int sockfd, uint16_t pid, uint8_t ttl, struct timeval *start_time);