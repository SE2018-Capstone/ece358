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
    fprintf(stderr, "Usage: %s server-ip server-port content-key\n", argv[0]);
    return -1;
  }

  struct sockaddr_in peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
  int peer_sockfd = connect_to_peer(peer_server);
  msg_add_content get_content_msg = {{LOOKUP_CONTENT}, 0, static_cast<unsigned int>(atoi(argv[3]))};

  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, &get_content_msg, sizeof(get_content_msg), 0)) < 0) {
    perror("lookupcontent send header"); return -1;
  }

  read_sock(peer_sockfd, (char *) &(get_content_msg.size), sizeof(unsigned long));

  if (get_content_msg.size == 0) {
    fprintf(stderr, "Error: no such content\n");
  } else {
    char content[get_content_msg.size];
    read_sock(peer_sockfd, content, sizeof(content));
    printf("%s\n", content);
  }

  if(close(peer_sockfd) < 0) {
    perror("close"); return -1;
  }
}
