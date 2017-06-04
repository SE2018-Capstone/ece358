#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <sstream>

#include "net_util.h"
#include "structs.h"

extern int debug_id;

peer_data pred;
peer_data succ;
int server_sockfd;
struct sockaddr_in server;
int state[MAX_STATES];
std::vector<pollfd> poll_fds;

unsigned int numContent = 0;
unsigned int numPeers = 1;
unsigned int nextId = 0;


// Note: Also opens a new connection with the given sockaddr
void update_peer_data(peer_data* data, struct sockaddr_in sockaddr, int peer_debug_id) {
  if (succ.peer_fd != pred.peer_fd && data->peer_fd != server_sockfd) {
    close(data->peer_fd);
    for (unsigned int j = 0; j < poll_fds.size(); j++) {
      if (poll_fds[j].fd == data->peer_fd) {
        poll_fds.erase(poll_fds.begin() + j);
      }
    }
  }

  data->peer_server = sockaddr;
  data->peer_debug_id = peer_debug_id;
  if (!sockaddr_equals(sockaddr, server)) {
    data->peer_fd = connect_to_peer(sockaddr);
    poll_fds.push_back({data->peer_fd, POLLIN, 0});
  } else {
    data->peer_fd = server_sockfd;
  }

  // If you used to be alone, update both pred and succ
  INFO("Setting %s to %i (fd: %i)\n", data == &pred ? "pred" : "succ", peer_debug_id, data->peer_fd);
  if (data == &pred && succ.peer_fd == server_sockfd && pred.peer_fd != server_sockfd) {
    memcpy(&succ, &pred, sizeof(peer_data));
    INFO("Also setting succ to %i\n", peer_debug_id);
  } else if (data == &succ && pred.peer_fd == server_sockfd && succ.peer_fd != server_sockfd) {
    memcpy(&pred, &succ, sizeof(peer_data));
    INFO("Also setting pred to %i\n", peer_debug_id);
  }
}

int main(int argc, char *argv[]) {
  srand ( time(NULL) );
  debug_id = rand() % 1000;
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
  pred = {server_sockfd, server, debug_id};
  succ = {server_sockfd, server, debug_id};

  fd_set_nonblocking(server_sockfd);
  if (listen(server_sockfd, 0) < 0) {
    perror("listen"); return -1;
  }

  poll_fds.push_back({server_sockfd, POLLIN, 0});

  if (peer_sockfd != -1) {
    poll_fds.push_back({peer_sockfd, POLLIN, 0});

    msg_basic get_info_msg = {{GET_INFO}};
    state[AWAITING_INSERTION] = 1;
    send_sock(peer_sockfd, (char *) &get_info_msg, sizeof(get_info_msg));

    msg_update_succ update_succ_msg = {{UPDATE_SUCC}, server, debug_id};
    send_sock(peer_sockfd, (char *) &update_succ_msg, sizeof(update_succ_msg));
  }

  int alive = 1;
  while(alive) {
    if (poll(&poll_fds[0], (nfds_t) poll_fds.size(), -1) == -1) {
      perror("poll"); return -1;
    }

    for (unsigned int i = 0; i < poll_fds.size(); i++) {
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
            INFO("Connection with %s was closed\n", sockfd_to_str(client_sockfd).c_str());
            close(client_sockfd); // Closed in update_peer_data
            poll_fds.erase(poll_fds.begin() + i);
          } else {
            INFO("Received %s message\n", msg_type_to_str(hdr.type));
            switch(hdr.type) {
              case UPDATE_SUCC: {
                msg_update_succ update_msg = {hdr};
                read_sock(client_sockfd, ((char *) &update_msg) + sizeof(msg_header),
                          sizeof(update_msg) - sizeof(msg_header));
                update_peer_data(&succ, update_msg.succ_server, update_msg.succ_debug_id);
                break;
              }
              case UPDATE_PRED: {
                msg_update_pred update_msg = {hdr};
                read_sock(client_sockfd, ((char *) &update_msg) + sizeof(msg_header),
                          sizeof(update_msg) - sizeof(msg_header));
                update_peer_data(&pred, update_msg.pred_server, update_msg.pred_debug_id);
                break;
              }
              case GET_INFO: {
                INFO("  Predecessor: %i, Successor: %i\n", pred.peer_debug_id, succ.peer_debug_id);
                msg_info msg = {{INFO}, server, debug_id, pred.peer_server, pred.peer_debug_id, succ.peer_server, succ.peer_debug_id, numContent, numPeers, nextId};
                send_sock(client_sockfd, (char *) &msg, sizeof(msg));
                break;
              }
              case INFO: {
                msg_info info_msg = {hdr};
                read_sock(client_sockfd, ((char *) &info_msg) + sizeof(msg_header),
                          sizeof(info_msg) - sizeof(msg_header));
                if (state[AWAITING_INSERTION] == 1) {
                  numContent = info_msg.numContent;
                  numPeers = info_msg.numPeers + 1;
                  nextId = info_msg.nextId;
                  pred = {client_sockfd, info_msg.server, info_msg.debug_id};
                  INFO("Setting pred to %i (fd: %i)\n", pred.peer_debug_id, pred.peer_fd);
                  if (!sockaddr_equals(pred.peer_server, info_msg.succ_server)) {
                    msg_update_pred update_pred_msg = {{UPDATE_PRED}, server, debug_id};
                    update_peer_data(&succ, info_msg.succ_server, info_msg.succ_debug_id);
                    send_sock(succ.peer_fd, (char*) &update_pred_msg, sizeof(update_pred_msg));
                  } else {
                    memcpy(&succ, &pred, sizeof(pred));
                  }
                  INFO("Setting succ to %i (fd: %i)\n", succ.peer_debug_id, succ.peer_fd);

                  state[AWAITING_COUNTS_RETURN] = 1;
                  msg_counts counts_msg = {{FORWARD_COUNTS}, numContent, numPeers, nextId};
                  send_sock(succ.peer_fd, (char*) &counts_msg, sizeof(counts_msg));

                  state[AWAITING_INSERTION] = false;
                }
                break;
              }
              case KILL: {
                if (pred.peer_fd != server_sockfd) { // Only false if only 1 peer
                  msg_update_pred update_pred_msg = {{UPDATE_PRED}, pred.peer_server, pred.peer_debug_id};
                  send_sock(succ.peer_fd, (char*) &update_pred_msg, sizeof(update_pred_msg));
                  msg_update_succ update_succ_msg = {{UPDATE_SUCC}, succ.peer_server, succ.peer_debug_id};
                  send_sock(pred.peer_fd, (char*) &update_succ_msg, sizeof(update_succ_msg));

                  msg_pred_removal pred_removal_msg = {{PRED_REMOVAL}};
                  send_sock(succ.peer_fd, (char*) &pred_removal_msg, sizeof(pred_removal_msg));
                }
                close(pred.peer_fd);
                close(succ.peer_fd);
                alive = 0;
                break;
              }
              case PRED_REMOVAL: {
                numPeers = numPeers - 1;

                state[AWAITING_COUNTS_RETURN] = 1;
                msg_counts counts_msg = {{FORWARD_COUNTS}, numContent, numPeers, nextId};
                send_sock(succ.peer_fd, (char*) &counts_msg, sizeof(counts_msg));
                break;
              }
              case FORWARD_COUNTS: {
                msg_counts counts_msg = {hdr};
                read_sock(client_sockfd, ((char *) &counts_msg) + sizeof(msg_header),
                          sizeof(counts_msg) - sizeof(msg_header));
                if (state[AWAITING_COUNTS_RETURN] == 1) {
                  INFO_YELLOW("Updated all counts:  numContent=%u, numPeers=%u, nextId=%u \n", numContent, numPeers, nextId);
                  state[AWAITING_COUNTS_RETURN] = 0;
                } else {
                  numContent = counts_msg.numContent;
                  numPeers = counts_msg.numPeers;
                  nextId = counts_msg.nextId;
                  send_sock(succ.peer_fd, (char*) &counts_msg, sizeof(counts_msg));
                }
                break;
              }
              case START_PING: {
                state[AWAITING_PING_RETURN] = 1;
                INFO("Server: %s   |   Pred: %3i  |  Succ: %3i\n", sockaddr_to_str(server).c_str(), pred.peer_debug_id, succ.peer_debug_id);
                if (succ.peer_fd != server_sockfd) {
                  msg_basic ping_msg = {{FORWARD_PING}};
                  send_sock(succ.peer_fd, (char*) &ping_msg, sizeof(ping_msg));
                }
                break;
              }
              case FORWARD_PING: {
                if (state[AWAITING_PING_RETURN] == 1) {
                  state[AWAITING_PING_RETURN] = 0;
                } else {
                  INFO("Server: %s   |   Pred: %3i  |  Succ: %3i\n", sockaddr_to_str(server).c_str(), pred.peer_debug_id, succ.peer_debug_id);
                  msg_basic ping_msg = {{FORWARD_PING}};
                  send_sock(succ.peer_fd, (char*) &ping_msg, sizeof(ping_msg));
                }
                break;
              }
              case ACK: {
                break;
              }
              default:
                INFO_RED("NO HANDLER FOR MSG\n");
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
