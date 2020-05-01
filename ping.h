#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <netinet/ip_icmp.h>
#include <fcntl.h>

#define PING_PACKET_SIZE 64
#define PORT_NO 0
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1

struct ping_packet {
  struct icmphdr header;
  char msg[PING_PACKET_SIZE-sizeof(struct icmphdr)];
};

char * dns_lookup(char * host_address, struct sockaddr_in * sock_address);
void send_ping(int ping_sockfd, struct sockaddr_in *ping_address, char * ping_ip, char *host);
unsigned short checksum(void *b, int len);
void int_handler(int dummy);
