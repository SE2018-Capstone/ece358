#include <string>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <sstream>

#include "net_util.h"
#include "structs.h"

std::vector<peer_data> peers;

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

  std::vector<pollfd> poll_fds;

  poll_fds.push_back({server_sockfd, POLLIN, 0});

  if (peer_sockfd != -1) {
    peers.push_back({peer_sockfd, peer_server});
    poll_fds.push_back({peer_sockfd, POLLIN, 0});

    msg_add_peer add_msg = {{ADD_PEER}, server};
    send_sock(peer_sockfd, (char *) &add_msg, sizeof(add_msg));
  }

  int alive = 1;
  while(alive) {
    if (poll(&poll_fds[0], (nfds_t) poll_fds.size(), -1) == -1) {
      perror("poll"); return -1;
    }

    for (int i = 0; i < poll_fds.size(); i++) {
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

          INFO("Connection accepted from %s\n", sockfd_to_str(client_sockfd).c_str());

          poll_fds.push_back({client_sockfd, POLLIN, 0});
        } else {
          // Existing peer is sending data
          int client_sockfd = poll_fds[i].fd;
          msg_header hdr;
          ssize_t recvlen;
          if((recvlen = read_sock(client_sockfd, (char *) &hdr, sizeof(msg_header))) == 0) {
            INFO("%s <-> %s connection was closed\n", sockfd_to_str(server_sockfd).c_str(), sockfd_to_str(client_sockfd).c_str());
            close(client_sockfd);
            poll_fds.erase(poll_fds.begin() + i);
          } else {
            INFO("%s received %s message\n", sockfd_to_str(server_sockfd).c_str(), msg_header_to_str(hdr));
            switch(hdr.type) {
              case ADD_PEER: {
                msg_add_peer add_msg = {hdr};
                read_sock(client_sockfd, ((char *) &add_msg) + sizeof(msg_header),
                          sizeof(msg_add_peer) - sizeof(msg_header));
                INFO("%s registering new peer at %s:%d\n", sockfd_to_str(server_sockfd).c_str(),
                     inet_ntoa(add_msg.sockaddr.sin_addr), ntohs(add_msg.sockaddr.sin_port));

                if (peers.size() > 0) {
                  msg_update_peers update_msg;
                  update_msg.hdr.type = UPDATE_PEERS;
                  update_msg.num_peers = (int) peers.size();
                  send_sock(client_sockfd, (char *) &update_msg, sizeof(update_msg));
                  send_sock(client_sockfd, (char *) &peers[0], (int) (peers.size() * sizeof(peer_data)));
                }
                peers.push_back({client_sockfd, add_msg.sockaddr});
                break;
              }
              case UPDATE_PEERS: {
                msg_update_peers update_msg = {hdr};
                read_sock(client_sockfd, ((char *) &update_msg) + sizeof(msg_header),
                          sizeof(msg_update_peers) - sizeof(msg_header));
                peer_data peer_update[update_msg.num_peers];
                read_sock(client_sockfd, (char *) &peer_update, sizeof(peer_update));
                for (int i = 0; i < update_msg.num_peers; i++) {
                  peers.push_back(peer_update[i]);
                }

                INFO("%s: NEW PEERS: \n", sockfd_to_str(server_sockfd).c_str());
                for (int i = 0; i < peers.size(); i++) {
                  INFO("-> %s:%d\n", inet_ntoa(peers[i].peer_server.sin_addr), ntohs(peers[i].peer_server.sin_port));
                }
                break;
              }
              case KILL: {
                close(client_sockfd);
                alive = 0;
                break;
              }
              default:
                exit(-1);
            }
          }
        }
      }
    }
  }

  shutdown(server_sockfd, SHUT_RDWR);
  INFO("%s:%d shutting down\n", argv[0], ntohs(server.sin_port));
  return 0;
}
