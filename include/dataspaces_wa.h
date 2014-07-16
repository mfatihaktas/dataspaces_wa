#ifndef _DSWA_H_
#define _DSWA_H_

#include "ds_client.h"

#define RI_MSG_SIZE 100

class WADspacesDriver
{
  public:
    WADspacesDriver(int app_id, int num_local_peers);
    ~WADspacesDriver();
  private:
    int app_id, num_local_peers;
    boost::shared_ptr<BCClient> bc_client_;
};

#endif //end of _DSWA_H_