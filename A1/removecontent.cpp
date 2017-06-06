//
// Created by Sam Maier on 2017-05-16.
//

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include "net_util.h"
#include "structs.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s server-ip server-port \"<contents>\"\n", argv[0]);
    return -1;
  }

  struct sockaddr_in peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
  int peer_sockfd = connect_to_peer(peer_server);
  msg_remove_content remove_content_msg = {{REMOVE_CONTENT}, atoi(argv[3]), true};
  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, &remove_content_msg, sizeof(remove_content_msg), 0)) < 0) {
    perror("removecontent send header"); return -1;
  }

  int is_success;
  read_sock(peer_sockfd, (char *) &is_success, sizeof(int));
  if (!is_success) {
    printf("Error: no such content");
  }

  if(shutdown(peer_sockfd, SHUT_RDWR) < 0) {
    perror("shutdown"); return -1;
  }
}
