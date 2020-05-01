#include "ping.h"

int ping_loop = 1;

// sockaddr is an IP socket address
char * dns_lookup(char * host_address, struct sockaddr_in * sock_address) {
  struct hostent * host_entity;
  int i;
  char * ip = (char*)malloc(NI_MAXHOST*sizeof(char));

  printf("\nResolving DNS...\n\n");

  host_entity = gethostbyname(host_address);
  // if null, no IP found
  if (!host_entity)
    return NULL;

  strcpy(ip, inet_ntoa(*(struct in_addr *) host_entity->h_addr));

  sock_address->sin_family = host_entity->h_addrtype; // address family
  sock_address->sin_port = htons (PORT_NO); // port in network byte order
  sock_address->sin_addr.s_addr = *(long*)host_entity->h_addr; // internet address

  return ip;
}

void send_ping(int ping_sockfd, struct sockaddr_in *ping_address, char * ping_ip, char *host) {
  // ttl static for now. hopefully make it up to the user
  int ttl = 64, msg_count = 0, i, address_len, flag = 1, msg_received_count = 0;

  struct ping_packet packet;
  struct sockaddr_in receive_address;
  struct timespec start_time, end_time, time1, time2;
  long double rtt_msec = 0, total_msec = 0;
  struct timeval tv_out;

  tv_out.tv_sec = RECV_TIMEOUT;
  tv_out.tv_usec = 0;

  clock_gettime(CLOCK_MONOTONIC, &time1);

  if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
    printf("\nSetting socket options to TTL failed!\n");
    return;
  }
  else {
    printf("Socket set to TTL...\n");
  }

  setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof tv_out);

  while(ping_loop)
  {
    flag = 1;

    bzero(&packet, sizeof(packet));

    packet.header.type = ICMP_ECHO;
    packet.header.un.echo.id = getpid();

    for (i = 0; i < sizeof(packet.msg)-1; ++i) {
      packet.msg[i] = i+'0';
    }

    packet.msg[i] = 0;
    packet.header.un.echo.sequence = ++msg_count;
    packet.header.checksum = checksum(&packet, sizeof(packet));

    usleep(PING_SLEEP_RATE);

    // send packet
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (sendto(ping_sockfd, &packet, sizeof(packet), 0, (struct sockaddr*) ping_address, 
      sizeof(*ping_address)) <= 0) {
      printf("Packet Sending Failed!\n");
      flag = 0;
    }
    else {
      clock_gettime(CLOCK_MONOTONIC, &end_time); 

      double timeElapsed = ((double)(end_time.tv_nsec - start_time.tv_nsec))/1000000.0;
      rtt_msec = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + timeElapsed; 

      // if packet was not sent, don't receive 
      if(flag) 
      { 
        if(!(packet.header.type == 8 && packet.header.code == 0))  
        { 
          printf("Error..Packet received with ICMP type %d code %d\n",  
              packet.header.type, packet.header.code); 
        } 
        else
        { 
          printf("%d bytes from h: %s (%s) msg_seq=%d ttl=%d rtt = %Lf ms.\n",  
              PING_PACKET_SIZE, host, ping_ip, msg_count, 
              ttl, rtt_msec); 

          msg_received_count++; 
        } 
      } 
    }     
  } 

  clock_gettime(CLOCK_MONOTONIC, &time2); 
  double timeElapsed = ((double)(time2.tv_nsec -  
        time1.tv_nsec))/1000000.0; 

  total_msec = (time2.tv_sec - time1.tv_sec)*1000.0 + timeElapsed;

  printf("\n===%s ping statistics===\n", ping_ip); 
  printf("\n%d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.\n\n",  
      msg_count, msg_received_count, 
      ((msg_count - msg_received_count)/msg_count) * 100.0, 
      total_msec);  

}

unsigned short checksum(void *b, int len) {
  unsigned short * buffer = b;
  unsigned int sum = 0;
  unsigned short result;

  for (sum = 0; len > 1; len -= 2)
    sum += *buffer++;

  // take care of odd byte if needed
  if (len == 1)
    sum += *(unsigned char*)buffer;

  // add back carry outs from top 16 bits to low 16 bits
  sum = (sum >> 16) + (sum & 0xFFFF); // add high 16 to low 16
  sum += (sum >> 16); // add carry
  result = ~sum; // truncate to 16 bits

  return result;
}

void int_handler(int dummy) {
  ping_loop = 0;
}
