#ifndef _DSWA_H_
#define _DSWA_H_

#include "ds_client.h"
#include "packet.h"
#include "remote_interact.h"

#define RI_MSG_SIZE 1000

class WADSDriver // Wide Area Dataspaces
{
  private:
    int app_id, base_client_id, num_local_peers;
    char data_id_t;
    
    int _app_id;
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<BCClient> bc_client_;
    MsgCoder msg_coder;
    patch_sdm::syncer<unsigned int> syncer;
    patch_sdm::thread_safe_map<std::string, char> data_id__ds_id_map;
  public:
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               char data_id_t);
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               char data_id_t, MPI_Comm mpi_comm);
    ~WADSDriver();
    
    int put(std::string key, unsigned int ver, std::string data_type, 
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(bool blocking, std::string key, unsigned int ver, std::string data_type,
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int handle_ri_reply(char* ri_reply);
};

#endif //end of _DSWA_H_