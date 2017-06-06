#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "net_util.h"
#include "structs.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s server-ip server-port\n", argv[0]);
    return -1;
  }

  struct sockaddr_in peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
  int peer_sockfd = connect_to_peer(peer_server);
  msg_basic get_keys_msg = {{GET_CONTENT_KEYS}};
  
  ssize_t sentlen;
  if ((sentlen = send(peer_sockfd, (char *) &get_keys_msg, sizeof(get_keys_msg), 0)) < 0) {
    perror("allkeys send header"); return -1;
  }

  int key_map_length;
  read_sock(peer_sockfd, (char *) &key_map_length, sizeof(int));

  if (key_map_length > 0) {
    unsigned int keys[key_map_length];
    read_sock(peer_sockfd, (char *) &keys, sizeof(keys));
    printf("%d", keys[0]);
    for (int i = 1; i < key_map_length; i++) {
      printf(" %d", keys[i]);
    }
  }
  printf("\n");

  if(close(peer_sockfd) < 0) {
    perror("close"); return -1;
  }
}
