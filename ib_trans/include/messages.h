#ifndef RDMA_MESSAGES_H
#define RDMA_MESSAGES_H

#include <stdlib.h>
#include <stdint.h>

const size_t BUFFER_SIZE = 10 * 10 * 1024 * 1024;

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