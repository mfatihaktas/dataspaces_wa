#ifndef _RDMA_MESSAGES_H_
#define _RDMA_MESSAGES_H_

#include <stdlib.h>
#include <stdint.h>

enum message_id
{
  MSG_INVALID = 0,
  MSG_MR,
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

#endif // _RDMA_MESSAGES_H_