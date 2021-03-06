//
// Created by Shiranka Miskin on 2017-05-16.
//

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include "net_util.h"
#include "structs.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s server-ip server-port\n", argv[0]);
    return -1;
  }

  struct sockaddr_in peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
  int peer_sockfd = connect_to_peer(peer_server);
  msg_basic ping_msg;
  memset(&ping_msg, 0, sizeof(msg_basic));
  ping_msg.hdr.type = START_PING;
  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, &ping_msg, sizeof(ping_msg), 0)) < 0) {
    perror("send ping"); return -1;
  }

  if(shutdown(peer_sockfd, SHUT_RDWR) < 0) {
    perror("shutdown"); return -1;
  }
}
