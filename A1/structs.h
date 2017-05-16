//
// Created by Shiranka Miskin on 2017-05-16.
//

#ifndef A1_STRUCTS_H
#define A1_STRUCTS_H

typedef enum {
  KILL,
  ADD_PEER,
  NUM_TYPES,
} msg_type;

typedef struct {
  msg_type type;
} msg_header;

typedef struct {
  msg_header hdr;
} msg_kill;

#endif //A1_STRUCTS_H
