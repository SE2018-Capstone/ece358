#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "net_util.h"
#include "structs.h"

char* msg_header_to_str(msg_header hdr) {
  switch(hdr.type) {
    case KILL: return "KILL";
    case ADD_PEER: return "ADD_PEER";
    default: return "Unknown message type";
  }
}


int main(int argc, char *argv[]) {
  int peer_sockfd = -1;
  if(argc == 3) {
    struct sockaddr_in peer_server;
    bzero(&peer_server, sizeof(struct sockaddr_in));
    peer_server.sin_family = AF_INET;
    if (!inet_aton(argv[1], &(peer_server.sin_addr))) {
      perror("invalid server-ip"); return -1;
    }
    peer_server.sin_port = htons(atoi(argv[2]));
    INFO("Connecting to peer at %s %d\n", inet_ntoa(peer_server.sin_addr), ntohs(peer_server.sin_port));

    if((peer_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("peer socket"); return -1;
    }
    if(connect(peer_sockfd, (struct sockaddr *)&peer_server, sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Error: no such peer\n");
      return -1;
    }
  } else {
    INFO("No peer provided, starting new host\n");
  }

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket"); return -1;
  }

  struct in_addr srvip;
  if((srvip.s_addr = getPublicIPAddr()) == 0) {
    fprintf(stderr, "getPublicIPAddr() returned error.\n");
    exit(-1);
  }

  struct sockaddr_in server;
  bzero(&server, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  memcpy(&(server.sin_addr), &srvip, sizeof(struct in_addr));
  server.sin_port = 0; // Allow OS to pick port


  if(mybind(sockfd, &server) < 0) {
    perror("mybind"); return -1;
  }

  socklen_t alen = sizeof(struct sockaddr_in);
  if (getsockname(sockfd, (struct sockaddr *)&server, &alen) < 0) {
    perror("getsockname"); return -1;
  }

  printf("%s %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

  for(int nconnections = 0; nconnections < 1; nconnections++) {
    if (listen(sockfd, 0) < 0) {
      perror("listen"); return -1;
    }

    int connectedsock;
    struct sockaddr_in client;
    alen = sizeof(struct sockaddr_in);
    if ((connectedsock = accept(sockfd, (struct sockaddr *)&client, &alen)) < 0) {
        perror("accept"); return -1;
    }

    INFO("Connection accepted from %s %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    msg_header hdr;
    ssize_t recvlen;
    if((recvlen = recv(connectedsock, &hdr, sizeof(msg_header), 0)) < 0) {
      perror("recv"); return -1;
    }

    INFO("Parent received %s message\n", msg_header_to_str(hdr));
    close(connectedsock);
  }

  close(sockfd);

  return 0;
}
