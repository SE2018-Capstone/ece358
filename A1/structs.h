//
// Created by Shiranka Miskin on 2017-05-16.
//

#ifndef A1_STRUCTS_H
#define A1_STRUCTS_H

typedef enum {
  KILL,
  ADD_PEER,
  UPDATE_PEERS,
  NUM_TYPES,
} msg_type;

typedef struct {
  msg_type type;
} msg_header;

typedef struct {
  msg_header hdr;
} msg_kill;

typedef struct {
  msg_header hdr;
  struct sockaddr_in sockaddr;
} msg_add_peer;

typedef struct {
  msg_header hdr;
  int num_peers;
  struct sockaddr_in* peers;
} msg_update_peers;

char* msg_header_to_str(msg_header hdr) {
  switch(hdr.type) {
    case KILL: return (char *) "KILL";
    case ADD_PEER: return (char *) "ADD_PEER";
    case UPDATE_PEERS: return (char *) "UPDATE_PEERS";
    default: return (char *) "UNKNOWN";
  }
}

typedef struct {
  int peer_fd;
  struct sockaddr_in peer_server;
} peer_data;


#endif //A1_STRUCTS_H
