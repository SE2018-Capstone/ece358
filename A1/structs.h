//
// Created by Shiranka Miskin on 2017-05-16.
//

#ifndef A1_STRUCTS_H
#define A1_STRUCTS_H
#include <arpa/inet.h>

typedef enum {
  KILL,
  UPDATE_SUCC,
  UPDATE_PRED,
  GET_NEIGHBOURS,
  NEIGHBOURS,
} msg_type;

// For debugging purposes
char* msg_type_to_str(msg_type type) {
  switch(type) {
    case KILL: return (char *) "KILL";
    case UPDATE_SUCC: return (char *) "UPDATE_SUCC";
    case UPDATE_PRED: return (char *) "UPDATE_PRED";
    case GET_NEIGHBOURS: return (char *) "GET_NEIGHBOURS";
    case NEIGHBOURS: return (char *) "NEIGHBOURS";
    default: return (char *) "UNKNOWN";
  }
}

typedef enum {
  REQUESTING_SUCCESSOR,
  MAX_STATES
} peer_states;

typedef struct {
  msg_type type;
} msg_header;

typedef struct {
  msg_header hdr;
} msg_kill;

typedef struct {
  msg_header hdr;
  struct sockaddr_in succ_server;
} msg_update_succ;

typedef struct {
  msg_header hdr;
  struct sockaddr_in pred_server;
} msg_update_pred;

typedef struct {
  msg_header hdr;
} msg_get_neighbours;

typedef struct {
  msg_header hdr;
  struct sockaddr_in pred_server;
  struct sockaddr_in succ_server;
} msg_neighbours;

typedef struct {
  int peer_fd;
  struct sockaddr_in peer_server;
} peer_data;


#endif //A1_STRUCTS_H
