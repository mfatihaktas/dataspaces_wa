#ifndef _DSWA_H_
#define _DSWA_H_

#include "ds_client.h"
#include "packet.h"

struct RMessenger
{
  public:
    RMessenger();
    ~RMessenger();
    std::map<std::string, std::string> gen_i_msg(std::string msg_type, int app_id, std::string var_name, 
                                                 unsigned int ver, int size, int ndim, 
                                                 uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
  private:
  
};

//TODO: a better way for syncing client and server of bccomm
#define WAIT_TIME_FOR_BCSERVER_DSLOCK 200*1000

#define RI_MSG_SIZE 1000

class WADspacesDriver
{
  public:
    WADspacesDriver(int app_id, int num_local_peers);
    ~WADspacesDriver();
    int local_put(std::string var_name, unsigned int ver, int size,
                  int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int remote_get(std::string var_name, unsigned int ver, int size,
                   int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
  private:
    int app_id, num_local_peers;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCClient> li_bc_client_;
    boost::shared_ptr<BCClient> ri_bc_client_;
    RMessenger rmessenger;
};

#endif //end of _DSWA_H_