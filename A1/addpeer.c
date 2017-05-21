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
  struct sockaddr_in peer_server;
  if(argc == 3) {
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
    fprintf(stderr, "getPublicIPAddr() returned error.\n"); exit(-1);
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

  fd_set_nonblocking(server_sockfd);
  if (listen(server_sockfd, 0) < 0) {
    perror("listen"); return -1;
  }

  int numfds = 1;
  poll_fds[0].fd = server_sockfd;
  poll_fds[0].events = POLLIN;

  if (peer_sockfd != -1) {
    poll_fds[numfds].fd = peer_sockfd;
    poll_fds[numfds].events = POLLIN;
    numfds++;

    msg_add_peer add_msg;
    bzero(&add_msg, sizeof(msg_add_peer));
    add_msg.hdr.type = ADD_PEER;
    add_msg.sockaddr = server;
    ssize_t sentlen;
    if ((sentlen = send(peer_sockfd, &add_msg, sizeof(add_msg), 0)) < 0) {
      perror("send add_peer"); return -1;
    }
  }

  int alive = 1;
  while(alive) {
    if (poll(poll_fds, numfds, -1) == -1) {
      perror("poll"); return -1;
    }

    for (int i = 0; i < numfds; i++) {
      if (poll_fds[i].revents & POLLIN) {
        poll_fds[i].revents = 0;
        if (poll_fds[i].fd == server_sockfd) {
          // New connection
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
          // Existing peer is sending data
          int client_sockfd = poll_fds[i].fd;
          msg_header hdr;
          bzero(&hdr, sizeof(msg_header));
          ssize_t recvlen;
          if((recvlen = recv(client_sockfd, &hdr, sizeof(msg_header), 0)) < 0) {
            perror("recv"); return -1;
          } else if (recvlen == 0) {
            INFO("%s:%d Connection was closed\n", argv[0], ntohs(server.sin_port));
            alive = 0; // Currently infinite loops, need to stop polling the fd
          } else {
            INFO("%s:%d received %s message\n", argv[0], ntohs(server.sin_port), msg_header_to_str(hdr));
            close(client_sockfd);
            alive = 0;
          }
        }
      }
    }
  }

  shutdown(server_sockfd, SHUT_RDWR);
  INFO("%s:%d shutting down\n", argv[0], ntohs(server.sin_port));
  return 0;
}
