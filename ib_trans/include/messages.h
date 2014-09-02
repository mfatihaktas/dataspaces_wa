#ifndef RDMA_MESSAGES_H
#define RDMA_MESSAGES_H

#include <stdlib.h>
#include <stdint.h>

enum message_id
{
  MSG_INVALID = 0,
  MSG_MR,
  MSG_READY,
  MSG_DONE
};

struct message
{
  int id;

  union
  {
    struct
    {
      uint64_t addr;
      uint32_t rkey;
    } mr;
  } data;
};

#endif