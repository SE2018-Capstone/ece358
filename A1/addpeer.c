#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>

#include "net_util.h"
#include "structs.h"

#define MAX_CONNECTIONS 256
struct pollfd poll_fds[MAX_CONNECTIONS];

char* msg_header_to_str(msg_header hdr) {
  switch(hdr.type) {
    case KILL: return "KILL";
    case ADD_PEER: return "ADD_PEER";
    default: return "UNKNOWN";
  }
}


int main(int argc, char *argv[]) {
  int peer_sockfd = -1;
  if(argc == 3) {
    struct sockaddr_in peer_server;
    peer_sockfd = connect_to_peer(argv[1], htons(atoi(argv[2])), &peer_server);
  } else {
    INFO("No peer provided, starting new host\n");
  }

  int server_sockfd;
  if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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


  if(mybind(server_sockfd, &server) < 0) {
    perror("mybind"); return -1;
  }

  socklen_t alen = sizeof(struct sockaddr_in);
  if (getsockname(server_sockfd, (struct sockaddr *)&server, &alen) < 0) {
    perror("getsockname"); return -1;
  }

  printf("%s %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

  daemonize(); // must run outside of the tty

  // Make non blocking
  int flags = fcntl(server_sockfd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl"); return -1;
  }
  flags |= O_NONBLOCK;
  if (fcntl(server_sockfd, F_SETFL, flags) == -1) {
    perror("fcntl"); return -1;
  }

  int numfds = 1;
  poll_fds[0].fd = server_sockfd;
  poll_fds[0].events = POLLIN;

  if (listen(server_sockfd, 0) < 0) {
    perror("listen"); return -1;
  }

  for(int nconnections = 0; nconnections < 1; nconnections++) {
    if (poll(poll_fds, numfds, -1) == -1) {
      perror("poll"); return -1;
    }

    for (int i = 0; i < numfds; i++) {
      if (poll_fds[i].revents & POLLIN) {
        poll_fds[i].revents = 0;
        if (poll_fds[i].fd == server_sockfd) {
          int client_sockfd;
          struct sockaddr_in client;
          alen = sizeof(struct sockaddr_in);
          if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&client, &alen)) < 0) {
              perror("accept"); return -1;
          }

          INFO("Connection accepted from %s %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
          poll_fds[numfds].fd = client_sockfd;
          poll_fds[numfds].events = POLLIN;
          numfds++;
        } else {
          int client_sockfd = poll_fds[i].fd;
          msg_header hdr;
          ssize_t recvlen;
          if((recvlen = recv(client_sockfd, &hdr, sizeof(msg_header), 0)) < 0) {
            perror("recv"); return -1;
          }

          INFO("%s received %s message\n", argv[0], msg_header_to_str(hdr));
          shutdown(client_sockfd, SHUT_RDWR);
        }
      }
    }
  }

  shutdown(server_sockfd, SHUT_RDWR);
  INFO("%s shutting down\n", argv[0]);
  return 0;
}
