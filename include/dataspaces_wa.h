#ifndef _DSWA_H_
#define _DSWA_H_

#include "remote_interact.h"

#define RI_MSG_SIZE 1000
/********************************************  WADSDriver  ****************************************/
class WADSDriver { // Wide Area Dataspaces
  private:
    int app_id, base_client_id, num_local_peers;
    DATA_ID_T data_id_t;
    
    int _app_id;
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<BCClient> bc_client_;
    patch_sdm::MsgCoder msg_coder;
    patch_all::syncer<unsigned int> syncer;
    patch_all::thread_safe_map<std::string, char> data_id__ds_id_map;
  public:
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               DATA_ID_T data_id_t);
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               DATA_ID_T data_id_t, MPI_Comm mpi_comm);
    ~WADSDriver();
    
    int put(std::string key, unsigned int ver, std::string data_type, 
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(bool blocking, std::string key, unsigned int ver, std::string data_type,
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int handle_ri_reply(char* ri_reply);
};

/***********************************  MWADSDriver : WADSDriver  ***********************************/
class MWADSDriver : public WADSDriver { // Markov
  public:
    MWADSDriver(int app_id, int base_client_id, int num_local_peers)
    : WADSDriver(app_id, base_client_id, num_local_peers, KV_DATA_ID)
    {
      LOG(INFO) << "MWADSDriver:: constructed.";
    }
    
    MWADSDriver(int app_id, int base_client_id, int num_local_peers, MPI_Comm mpi_comm)
    : WADSDriver(app_id, base_client_id, num_local_peers, KV_DATA_ID, mpi_comm)
    {
      LOG(INFO) << "MWADSDriver:: constructed.";
    }
    
    ~MWADSDriver() { LOG(INFO) << "MWADSDriver:: destructed."; }
};

/***********************************  SWADSDriver : WADSDriver  ***********************************/
class SWADSDriver : public WADSDriver { // Spatial
  public:
    SWADSDriver(int app_id, int base_client_id, int num_local_peers)
    : WADSDriver(app_id, base_client_id, num_local_peers, LUCOOR_DATA_ID)
    {
      LOG(INFO) << "SWADSDriver:: constructed.";
    }
    
    SWADSDriver(int app_id, int base_client_id, int num_local_peers, MPI_Comm mpi_comm)
    : WADSDriver(app_id, base_client_id, num_local_peers, LUCOOR_DATA_ID, mpi_comm)
    {
      LOG(INFO) << "SWADSDriver:: constructed.";
    }
    
    ~SWADSDriver() { LOG(INFO) << "SWADSDriver:: destructed."; }
};

#endif //end of _DSWA_H_