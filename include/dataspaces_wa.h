#ifndef _DSWA_H_
#define _DSWA_H_

#include "ds_client.h"
#include "packet.h"

struct RMessenger
{
  public:
    RMessenger();
    ~RMessenger();
    std::map<std::string, std::string> gen_i_msg(std::string msg_type, int app_id,
                                                 std::string data_type, std::string key,
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
    WADspacesDriver(MPI_Comm mpi_comm, int app_id, int num_local_peers);
    ~WADspacesDriver();
    void print_str_map(std::map<std::string, std::string> str_map);
    
    int put(std::string data_type, std::string key, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(bool blocking, std::string data_type, std::string key, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int handle_ri_reply(char* ri_reply);
  private:
    int app_id, num_local_peers;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCClient> bc_client_;
    RMessenger rmessenger;
    IMsgCoder imsg_coder;
    syncer<key_ver_pair> rg_syncer; //remote_get_syncer
    
    thread_safe_map<key_ver_pair, char> key_ver__dsid_map; //for getting rq_reply
};

#endif //end of _DSWA_H_