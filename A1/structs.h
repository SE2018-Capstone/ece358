//
// Created by Shiranka Miskin on 2017-05-16.
//

#ifndef A1_STRUCTS_H
#define A1_STRUCTS_H
#include <arpa/inet.h>

typedef struct {
  int peer_fd;
  struct sockaddr_in peer_server;
  int peer_debug_id;
} peer_data;

typedef enum {
  KILL,
  UPDATE_SUCC,
  UPDATE_PRED,
  GET_INFO,
  INFO,
  START_PING,
  FORWARD_PING,
  FORWARD_COUNTS,
  PRED_REMOVAL,
  ADD_CONTENT,
  LOOKUP_CONTENT,
  FORWARD_LOOKUP_CONTENT,
  GET_CONTENT_KEYS,
  REMOVE_CONTENT,
  REMOVE_FINISHED,
  FORWARD_CONTENT,
  REQUEST_CONTENT,
  ADD_CONTENT_FINISHED,
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
    case FORWARD_COUNTS: return (char *) "FORWARD_COUNTS";
    case PRED_REMOVAL: return (char *) "PRED_REMOVAL";
    case ADD_CONTENT: return (char *) "ADD_CONTENT";
    case FORWARD_LOOKUP_CONTENT: return (char *) "FORWARD_LOOKUP_CONTENT";
    case LOOKUP_CONTENT: return (char *) "LOOKUP_CONTENT";
    case GET_CONTENT_KEYS: return (char *) "GET_CONTENT_KEYS";
    case REMOVE_CONTENT: return (char *) "REMOVE_CONTENT";
    case REMOVE_FINISHED: return (char *) "REMOVE_FINISHED";
    case FORWARD_CONTENT: return (char *) "FORWARD_CONTENT";
    case REQUEST_CONTENT: return (char *) "REQUEST_CONTENT";
    case ADD_CONTENT_FINISHED: return (char *) "ADD_CONTENT_FINISHED";
    case ACK: return (char *) "ACK";
    default: return (char *) "UNKNOWN";
  }
}

typedef enum {
  AWAITING_INSERTION,
  AWAITING_PING_RETURN,
  AWAITING_COUNTS_RETURN,
  AWAITING_LOOKUPCONTENT_RETURN,
  AWAITING_REMOVE_RETURN,
  AWAITING_ADDCONTENT_RETURN,
  REMOVER,
  REQUESTING_CONTENT,
  MAX_STATES
} peer_states;

typedef enum {
  ADDCONTENT_SOCKFD,
  LOOKUPCONTENT_SOCKFD,
  REMOVECONTENT_SOCKFD,
  MAX_STATE_DATA,
} peer_state_data;

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
  unsigned int numContent;
  unsigned int numPeers;
  unsigned int nextId;
} msg_info;

typedef struct {
  msg_header hdr;
  unsigned int numContent;
  unsigned int numPeers;
  unsigned int nextId;
} msg_counts;

typedef struct {
  msg_header hdr;
// Hold all the content that used to be on this peer
} msg_pred_removal;

typedef struct {
  msg_header hdr;
  unsigned long size;
  unsigned int id;
  bool firstRequest;
} msg_add_content;

typedef struct {
  msg_header hdr;
  unsigned long size;
  unsigned int id;
} msg_forward_content;

typedef struct {
  msg_header hdr;
  unsigned long size;
  unsigned int id;
} msg_get_content;

typedef struct {
  msg_header hdr;
  unsigned int numRequested;
  unsigned int reservedRequests;
} msg_request_content;

typedef struct {
  msg_header hdr;
  unsigned int id;
  bool firstRequest;
} msg_remove_content;

typedef struct {
  msg_header hdr;
  unsigned int id;
} msg_addcontent_return;

typedef struct {
  msg_header hdr;
} msg_remove_finished;

#endif //A1_STRUCTS_H
