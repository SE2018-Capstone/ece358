#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <sstream>
#include <map>

#include "net_util.h"
#include "structs.h"

extern int debug_id;

peer_data pred;
peer_data succ;
int server_sockfd;
struct sockaddr_in server;
int state[MAX_STATES];
int data[MAX_STATE_DATA];
std::vector<pollfd> poll_fds;

unsigned int zero = 0;
unsigned int one = 0;

unsigned int numContent = 0;
unsigned int numPeers = 1;
unsigned int nextId = 1; // Id of 0 is reserved to determine whether msg_add_content is being forwarded or not
std::map<unsigned int, std::string> content_map;

// Dont need floor since n/d is floor
unsigned int uint_ceil(unsigned int n, unsigned int d) {
  return (n % d) ? n / d + 1 : n / d;
}

void print_info() {
#ifdef DEBUG
  fprintf(stderr, "\x1b[34m      Pred: %i, Succ: %i, Content: [", pred.peer_debug_id, succ.peer_debug_id);
  for (std::map<unsigned int, std::string>::iterator it = content_map.begin(); it != content_map.end();) {
    fprintf(stderr, "(%i, %.3s%s)", it->first, (it->second).c_str(), (it->second).size() > 3 ? "…" : "");
    if (++it != content_map.end()) fprintf(stderr, ", ");
  }
  fprintf(stderr, "]\n\x1b[0m");
#endif
}

void forward_counts() {
  if (succ.peer_fd == server_sockfd) return;
  state[AWAITING_COUNTS_RETURN] = 1;
  msg_counts counts_msg = {{FORWARD_COUNTS}, numContent, numPeers, nextId};
  send_sock(succ.peer_fd, (char*) &counts_msg, sizeof(counts_msg));
}

void forward_content(msg_forward_content msg, const char* content) {
  if (send(succ.peer_fd, &msg, sizeof(msg), 0) < 0) {
    perror("forward_content send header"); exit(-1);
  }
  if (send(succ.peer_fd, content, msg.size, 0) < 0) {
    perror("forward_content send content"); exit(-1);
  }
}

void forward_add_content(msg_add_content msg, const char* content) {
  if (send(succ.peer_fd, &msg, sizeof(msg), 0) < 0) {
    perror("forward_content send header"); exit(-1);
  }
  if (send(succ.peer_fd, content, msg.size, 0) < 0) {
    perror("forward_content send content"); exit(-1);
  }
}

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
    poll_fds.push_back(pollfd{data->peer_fd, POLLIN, 0});
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
  debug_id = rand() % 980 + 20;
  memset(&state, 0, sizeof(state));
  memset(&data, 0, sizeof(state));

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

  pred = {server_sockfd, server, debug_id};
  succ = {server_sockfd, server, debug_id};

  fd_set_nonblocking(server_sockfd);
  if (listen(server_sockfd, 0) < 0) {
    perror("listen"); return -1;
  }

  poll_fds.push_back(pollfd{server_sockfd, POLLIN, 0});

  if (peer_sockfd == -1) {
    daemonize(); // Job done, end addpeer.cpp
  } else {
    // Begin process to connect to peers.  Once connected, updated counts will be forwarded,
    // and new content will be sent to this new peer.  Once that entire process is done,
    // daemonize() will be called to end addpeer.cpp.
    poll_fds.push_back(pollfd{peer_sockfd, POLLIN, 0});

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

          poll_fds.push_back(pollfd{client_sockfd, POLLIN, 0});
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
              case ADD_CONTENT: {
                msg_add_content content_msg = {hdr};
                read_sock(client_sockfd, ((char *) &content_msg) + sizeof(msg_header),
                          sizeof(content_msg) - sizeof(msg_header));
                char content[content_msg.size];
                read_sock(client_sockfd, content, sizeof(content));

                if (content_msg.firstRequest) {
                  state[AWAITING_ADDCONTENT_RETURN] =  1;
                  data[ADDCONTENT_SOCKFD] = client_sockfd;
                  numContent++;
                  content_msg.id = nextId++;
                  forward_counts();
                  INFO_YELLOW("Received content %i: %s\n", content_msg.id, content);
                }

                unsigned int maxItemsHeld = uint_ceil(numContent, numPeers);
                if (content_map.size() < maxItemsHeld) {
                  content_map[content_msg.id] = "";
                  content_map[content_msg.id].assign(content, sizeof(content));
                  INFO_YELLOW("Consumed content %d\n", content_msg.id);
                  msg_addcontent_return return_msg = {{ADD_CONTENT_FINISHED}, content_msg.id};
                  send_sock(succ.peer_fd, (char*) &return_msg, sizeof(return_msg));
                } else {
                  content_msg.firstRequest = false;
                  forward_add_content(content_msg, content);
                }
                break;
              }
              case ADD_CONTENT_FINISHED: {
                msg_addcontent_return return_msg = {hdr};
                read_sock(client_sockfd, ((char *) &return_msg) + sizeof(msg_header),
                          sizeof(return_msg) - sizeof(msg_header));
                if (state[AWAITING_ADDCONTENT_RETURN] > 0) {
                  state[AWAITING_ADDCONTENT_RETURN] = 0;
                  INFO_GREEN("ADDCONTENT FINISHED\n");
                  send_sock(data[ADDCONTENT_SOCKFD], (char *) &(return_msg.id), sizeof(return_msg.id));
                } else {
                  send_sock(succ.peer_fd, (char *) &(return_msg), sizeof(return_msg));
                }
                break;
              }
              case GET_CONTENT_KEYS: {
                int map_size = content_map.size();
                send_sock(client_sockfd, (char *) &map_size, sizeof(int));
                if (map_size > 0) {
                  unsigned int keys[map_size];
                  int i = 0;
                  for (std::map<unsigned int, std::string>::iterator it = content_map.begin(); it != content_map.end(); it++) {
                    keys[i++] = it->first;
                  }
                  send_sock(client_sockfd, (char *) &keys, sizeof(keys));
                }
                break;
              }
              case REMOVE_FINISHED: {
                msg_remove_finished remove_msg = {hdr};
                read_sock(client_sockfd, ((char *) &remove_msg) + sizeof(msg_header),
                          sizeof(remove_msg) - sizeof(msg_header));
                if (state[AWAITING_REMOVE_RETURN]) {
                  state[AWAITING_REMOVE_RETURN] = 0;
                  INFO_GREEN("REMOVECONTENT FINISHED");
                  send_sock(data[REMOVECONTENT_SOCKFD], (char *) &one, sizeof(one));
                } else {
                  send_sock(succ.peer_fd, (char*) &remove_msg, sizeof(remove_msg));
                }
                break;
              }
              case REMOVE_CONTENT: {
                msg_remove_content remove_msg = {hdr};
                read_sock(client_sockfd, ((char *) &remove_msg) + sizeof(msg_header),
                          sizeof(remove_msg) - sizeof(msg_header));
                if (remove_msg.firstRequest) {
                  state[AWAITING_REMOVE_RETURN] = 1;
                  data[REMOVECONTENT_SOCKFD] = client_sockfd;
                  INFO_YELLOW("Waiting for remove to return\n");
                } else if (state[AWAITING_REMOVE_RETURN]) {
                  // The lookup for the item failed
                  // Report error to client
                  INFO_YELLOW("Remove couldnt find anything\n");
                  state[AWAITING_REMOVE_RETURN] = 0;
                  INFO_GREEN("REMOVECONTENT FINISHED");
                  send_sock(data[REMOVECONTENT_SOCKFD], (char *) &zero, sizeof(zero));
                  break;
                }
                if (content_map.count(remove_msg.id)) {
                  // This peer has the content to be removed
                  content_map.erase(remove_msg.id);
                  INFO_YELLOW("Removed content %i\n", remove_msg.id);
                  numContent--;
                  forward_counts();
                  if (content_map.size() < numContent/numPeers) {
                    state[REQUESTING_CONTENT] = 1;
                    state[REMOVER] = 1;
                    // Request 1 content since we are under floor
                    unsigned int numRequests = 1;
                    unsigned int reservedRequests = (uint_ceil(numContent+1,numPeers)-uint_ceil(numContent,numPeers))*((numContent+1)%numPeers);
                    INFO("Requesting %i, with %i reserved\n", numRequests, reservedRequests);
                    msg_request_content request_msg = {{REQUEST_CONTENT}, numRequests, reservedRequests};
                    send_sock(succ.peer_fd, (char*) &request_msg, sizeof(request_msg));
                  } else {
                    msg_remove_finished finished = {{REMOVE_FINISHED}};
                    send_sock(succ.peer_fd, (char*) &finished, sizeof(finished));
                  }
                } else {
                  // Pass on remove content request
                  remove_msg.firstRequest = false;
                  send_sock(succ.peer_fd, (char*) &remove_msg, sizeof(remove_msg));
                }
                break;
              }
              case GET_INFO: {
                print_info();
                msg_info msg = {{INFO}, server, debug_id, pred.peer_server, pred.peer_debug_id, succ.peer_server, succ.peer_debug_id, numContent, numPeers, nextId};
                send_sock(client_sockfd, (char *) &msg, sizeof(msg));
                break;
              }
              case INFO: {
                msg_info info_msg = {hdr};
                read_sock(client_sockfd, ((char *) &info_msg) + sizeof(msg_header),
                          sizeof(info_msg) - sizeof(msg_header));
                if (state[AWAITING_INSERTION] == 1) {
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

                  numContent = info_msg.numContent;
                  numPeers = info_msg.numPeers;
                  nextId = info_msg.nextId;
                  unsigned int numRequested = numContent / (numPeers + 1);
                  unsigned int numAboveMin = numContent % numPeers;
                  unsigned int numMin = numPeers - numAboveMin;
                  unsigned int maxsAboveMax = (uint_ceil(numContent, numPeers) - uint_ceil(numContent, numPeers + 1)) * numAboveMin;
                  unsigned int minsAboveMax = ((numContent / numPeers) - uint_ceil(numContent, numPeers + 1)) * numMin;
                  unsigned int reservedRequests = maxsAboveMax + ((minsAboveMax > 0) ? minsAboveMax : 0);
                  numPeers = numPeers + 1;

                  forward_counts();
                  if (numRequested > 0) {
                    state[REQUESTING_CONTENT] = numRequested;
                    msg_request_content request_msg = {{REQUEST_CONTENT}, numRequested, reservedRequests};
                    send_sock(succ.peer_fd, (char*) &request_msg, sizeof(request_msg));
                  }
                }
                break;
              }
              case REQUEST_CONTENT: {
                msg_request_content request_msg = {hdr};
                read_sock(client_sockfd, ((char *) &request_msg) + sizeof(msg_header),
                          sizeof(request_msg) - sizeof(msg_header));
                unsigned int numItemsIHold = content_map.size();
                std::map<unsigned int, std::string>::iterator it = content_map.begin();

                int shouldGoToFloor = request_msg.numRequested - request_msg.reservedRequests > 0 && numItemsIHold > (numContent / numPeers);
                int numItemsToDonate = content_map.size() - uint_ceil(numContent, numPeers) + shouldGoToFloor;
                for (int i = 0; i < numItemsToDonate; i++) {
                  unsigned int key = it->first;
                  std::string val = it->second;
                  content_map.erase(it++);
                  unsigned int size = (unsigned int) val.size();
                  msg_forward_content forward_msg = {{FORWARD_CONTENT}, size, key};
                  forward_content(forward_msg, val.c_str());
                  request_msg.numRequested--;
                  if (i < numItemsIHold - uint_ceil(numContent, numPeers)) {
                    request_msg.reservedRequests--;
                  }
                }

                if (request_msg.numRequested > 0) {
                  send_sock(succ.peer_fd, (char*) &request_msg, sizeof(request_msg));
                }
                break;
              }
              case FORWARD_CONTENT: {
                msg_forward_content content_msg = {hdr};
                read_sock(client_sockfd, ((char *) &content_msg) + sizeof(msg_header),
                          sizeof(content_msg) - sizeof(msg_header));
                char content[content_msg.size];
                read_sock(client_sockfd, content, sizeof(content));

                if (state[REQUESTING_CONTENT] > 0) {
                  content_map[content_msg.id] = "";
                  content_map[content_msg.id].assign(content, sizeof(content));
                  INFO_YELLOW("Consumed forwarded content %d\n", content_msg.id);
                  state[REQUESTING_CONTENT]--;
                  if (!state[REQUESTING_CONTENT] && state[REMOVER]) {
                    state[REMOVER] = 0;
                    msg_remove_finished finished = {{REMOVE_FINISHED}};
                    send_sock(succ.peer_fd, (char*) &finished, sizeof(finished));
                  }
                  if (!state[REQUESTING_CONTENT] && state[AWAITING_INSERTION]) {
                    state[AWAITING_INSERTION] = 0;
                    daemonize();
                  }
                } else {
                  forward_content(content_msg, content);
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
                  if (state[AWAITING_INSERTION] && !state[REQUESTING_CONTENT]) {
                    state[AWAITING_INSERTION] = 0;
                    daemonize(); // Can finally end addpeer.cpp's main process
                  }
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
                print_info();
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
                  print_info();
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
