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
  GET_INFO,
  INFO,
  START_PING,
  FORWARD_PING,
  ACK, // No purpose, just debugging
} msg_type;

// For debugging purposes
char* msg_type_to_str(msg_type type) {
  switch(type) {
    case KILL: return (char *) "KILL";
    case UPDATE_SUCC: return (char *) "UPDATE_SUCC";
    case UPDATE_PRED: return (char *) "UPDATE_PRED";
    case GET_INFO: return (char *) "GET_INFO";
    case INFO: return (char *) "INFO";
    case START_PING: return (char *) "START_PING";
    case FORWARD_PING: return (char *) "FORWARD_PING";
    case ACK: return (char *) "ACK";
    default: return (char *) "UNKNOWN";
  }
}

typedef enum {
  AWAITING_INSERTION,
  AWAITING_PING_RETURN,
  MAX_STATES
} peer_states;

typedef struct {
  msg_type type;
} msg_header;

typedef struct {
  msg_header hdr;
} msg_basic;

typedef struct {
  msg_header hdr;
  struct sockaddr_in succ_server;
  int succ_debug_id; // DEBUG
} msg_update_succ;

typedef struct {
  msg_header hdr;
  struct sockaddr_in pred_server;
  int pred_debug_id;
} msg_update_pred;

typedef struct {
  msg_header hdr;
  struct sockaddr_in server;
  int debug_id;
  struct sockaddr_in pred_server;
  int pred_debug_id;
  struct sockaddr_in succ_server;
  int succ_debug_id;
} msg_info;

typedef struct {
  int peer_fd;
  struct sockaddr_in peer_server;
  int peer_debug_id;
} peer_data;


#endif //A1_STRUCTS_H
