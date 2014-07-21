#ifndef _DSWA_H_
#define _DSWA_H_

#include "ds_client.h"

#define RI_MSG_SIZE 100

struct RMessenger
{
  public:
    RMessenger();
    ~RMessenger();
    std::map<std::string, std::string> gen_remote_get(int app_id, std::string var_name, unsigned int ver,
                                                      int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
  private:
  
};

class WADspacesDriver
{
  public:
    WADspacesDriver(int app_id, int num_local_peers);
    ~WADspacesDriver();
    int remote_get(std::string var_name, unsigned int ver, int size,
                   int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
  private:
    int app_id, num_local_peers;
    boost::shared_ptr<BCClient> bc_client_;
    RMessenger rmessenger;
};

#endif //end of _DSWA_H_