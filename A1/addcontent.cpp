//
// Created by Shiranka Miskin on 2017-05-16.
//

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "net_util.h"
#include "structs.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s server-ip server-port \"<contents>\"\n", argv[0]);
    return -1;
  }

  struct sockaddr_in peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
  int peer_sockfd = connect_to_peer(peer_server);
  msg_add_content add_content_msg = {{ADD_CONTENT}, strlen(argv[3]) + 1, 0};
  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, &add_content_msg, sizeof(add_content_msg), 0)) < 0) {
    perror("addcontent send header"); return -1;
  }

  if ((sentlen = send(peer_sockfd, argv[3], add_content_msg.size, 0)) < 0) {
    perror("addcontent send content"); return -1;
  }

  read_sock(peer_sockfd, (char *) &(add_content_msg.id), sizeof(unsigned int));
  printf("%d\n", add_content_msg.id);

  if(close(peer_sockfd) < 0) {
    perror("close"); return -1;
  }
}
