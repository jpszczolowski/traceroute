#define _BSD_SOURCE

#include <stdint.h>
#include <sys/time.h>

#define handle_error(msg) do { perror(msg " error"); exit(EXIT_FAILURE); \
                          } while (0)

void send_single_icmp(int sockfd, const char *ip, uint16_t id, uint16_t sequence, int ttl);
int wait_for_icmps(int sockfd, uint16_t pid, uint8_t ttl, struct timeval *start_time,
                   struct timeval *end_time, int nqueries);