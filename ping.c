/*
 * Vivian Killian, vkillian@pdx.edu, April 2020
 * this a ping program
 * tutorials/references:
 * https://www.geeksforgeeks.org/ping-in-c/
 * ohm.bu.edu/~cdubois/CLEAN/pings%20and%20things/pinger.c
 *
 * compile: gcc *.c -o my_ping
 * run: sudo ./my_ping <hostname>
 *
 * Algorithm:
 * 1. parse args (hostname)
 * 2. DNS lookup - gethostname() is used to convert human website to IP address.
 * 3. generate ICMP echo request
 * 4. start timer
 * 5. send ICMP echo request (infinite loop unless TTL)
 * 6. if get reply -> stop timer and report time
 * 6b. if timer expires, report error
*/

#include "ping.h"

int main(int argc, char ** argv) {
  char * ip_address;
  int sockfd; // socket file descriptor
  struct sockaddr_in sock_address;

  if (getuid() != 0) {
	printf("\n%s: Root privileges needed. Run this with sudo!\n\n", argv[0]);
	return 0;
  }

  if(argc<2) 
    { 
        printf("\nOops! The format is %s <address>\n\n", argv[0]); 
        return 0; 
    } 

  ip_address = dns_lookup(argv[1], &sock_address);
  if (!ip_address) {
    printf("\nDNS lookup failed! Could not resolve hostname!\n\n");
    return 0;
  }

  printf("Trying to connect to '%s' IP: %s\n", argv[1], ip_address);

  // socket
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd<0) {
    printf("\nSocket file descriptor not received!!\n");
    return 0;
  }
  else
    printf("\nSocket file descriptor %d received\n", sockfd);

  signal(SIGINT, int_handler); // catching interrupt

  // send pings
  send_ping(sockfd, &sock_address, ip_address, argv[1]);

  return 0;
}
