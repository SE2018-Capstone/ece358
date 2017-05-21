//
// Created by Shiranka Miskin on 2017-05-16.
//

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

  struct sockaddr_in peer_server;
  int peer_sockfd = connect_to_peer(argv[1], htons(atoi(argv[2])), &peer_server);
  msg_kill kill_msg;
  bzero(&kill_msg, sizeof(msg_kill));
  kill_msg.hdr.type = KILL;
  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, &kill_msg, sizeof(msg_kill), 0)) < 0) {
    perror("send kill"); return -1;
  }

  if(shutdown(peer_sockfd, SHUT_RDWR) < 0) {
    perror("shutdown"); return -1;
  }
}
