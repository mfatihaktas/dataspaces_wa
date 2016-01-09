#ifndef _IB_MSG_H_
#define _IB_MSG_H_

#include <stdlib.h>
#include <stdint.h>

enum message_id {
  MSG_INVALID = 0,
  MSG_MR,
  MSG_READY_TO_RECV,
  MSG_DONE
};

struct message {
  int id;

  union {
    struct {
      uint64_t addr;
      uint32_t rkey;
    } mr;
  } data;
};

#endif // _IB_MSG_H_