#include <string>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <sstream>

#include "net_util.h"
#include "structs.h"

peer_data pred;
peer_data succ;
int server_sockfd;
int state[MAX_STATES];
std::vector<pollfd> poll_fds;

void update_peer_data(peer_data* data, struct sockaddr_in sockaddr) {
  if (data->peer_fd != server_sockfd) {
    close(data->peer_fd);
    for (int j = 0; j < poll_fds.size(); j++) {
      if (poll_fds[j].fd == data->peer_fd) {
        poll_fds.erase(poll_fds.begin() + j);
      }
    }
  }

  data->peer_server = sockaddr;
  data->peer_fd = connect_to_peer(sockaddr);
  poll_fds.push_back({data->peer_fd, POLLIN, 0});
}

int main(int argc, char *argv[]) {
  memset(&state, 0, sizeof(state));

  int peer_sockfd = -1;
  struct sockaddr_in peer_server;
  if(argc == 3) {
    peer_server = create_sockaddr_in(argv[1], htons(atoi(argv[2])));
    peer_sockfd = connect_to_peer(peer_server);
  } else {
    INFO("No peer provided, starting new host\n");
  }

  if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket"); return -1;
  }

  struct in_addr srvip;
  if((srvip.s_addr = getPublicIPAddr()) == 0) {
    fprintf(stderr, "getPublicIPAddr() returned error.\n"); exit(-1);
  }

  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
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
  pred = {server_sockfd, server};
  succ = {server_sockfd, server};

  fd_set_nonblocking(server_sockfd);
  if (listen(server_sockfd, 0) < 0) {
    perror("listen"); return -1;
  }

  poll_fds.push_back({server_sockfd, POLLIN, 0});

  if (peer_sockfd != -1) {
    poll_fds.push_back({peer_sockfd, POLLIN, 0});

    msg_update_pred get_neighbors_msg = {{GET_NEIGHBOURS}};
    state[REQUESTING_SUCCESSOR] = 1;
    send_sock(peer_sockfd, (char *) &get_neighbors_msg, sizeof(get_neighbors_msg));

    msg_update_pred update_pred_msg = {{UPDATE_PRED}, server};
    send_sock(peer_sockfd, (char *) &update_pred_msg, sizeof(update_pred_msg));
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
          if(read_sock(client_sockfd, (char *) &hdr, sizeof(msg_header)) == 0) {
            INFO("%s <-> %s connection was closed\n", sockfd_to_str(server_sockfd).c_str(), sockfd_to_str(client_sockfd).c_str());
            close(client_sockfd);
            poll_fds.erase(poll_fds.begin() + i);
          } else {
            INFO("%s received %s message\n", sockfd_to_str(server_sockfd).c_str(), msg_type_to_str(hdr.type));
            switch(hdr.type) {
              case UPDATE_SUCC: {
                msg_update_succ update_msg = {hdr};
                read_sock(client_sockfd, ((char *) &update_msg) + sizeof(msg_header),
                          sizeof(update_msg) - sizeof(msg_header));
                update_peer_data(&succ, update_msg.succ_server);
                break;
              }
              case UPDATE_PRED: {
                msg_update_pred update_msg = {hdr};
                read_sock(client_sockfd, ((char *) &update_msg) + sizeof(msg_header),
                          sizeof(update_msg) - sizeof(msg_header));
                update_peer_data(&succ, update_msg.pred_server);
                break;
              }
              case GET_NEIGHBOURS: {
                msg_neighbours msg = {{NEIGHBOURS}, pred.peer_server, succ.peer_server};
                send_sock(client_sockfd, (char *) &msg, sizeof(msg));
                break;
              }
              case NEIGHBOURS: {
                msg_neighbours neighbours_msg = {hdr};
                read_sock(client_sockfd, ((char *) &neighbours_msg) + sizeof(msg_header),
                          sizeof(neighbours_msg) - sizeof(msg_header));
                if (state[REQUESTING_SUCCESSOR] == 1) {
                  update_peer_data(&succ, neighbours_msg.succ_server);
                  msg_update_pred update_pred_msg = {{UPDATE_PRED}, server};
                  send_sock(succ.peer_fd, (char*) &update_pred_msg, sizeof(update_pred_msg));
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
