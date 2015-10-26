#ifndef _DATASPACES_WA_H_
#define _DATASPACES_WA_H_

#include "remote_interact.h"

#define RI_MSG_SIZE 1000
/********************************************  WADSDriver  ****************************************/
class WADSDriver { // Wide Area Dataspaces
  private:
    int app_id, base_client_id, num_local_peers;
    DATA_ID_T data_id_t;
    
    int _app_id;
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<SDMNode> lsdm_node_;
    patch_sdm::MsgCoder msg_coder;
    patch_all::syncer<unsigned int> syncer;
    patch_all::thread_safe_map<std::string, int> data_id__ds_id_map;
  public:
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               DATA_ID_T data_id_t,
               std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport);
    WADSDriver(int app_id, int base_client_id, int num_local_peers,
               DATA_ID_T data_id_t, MPI_Comm& mpi_comm,
               std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport);
    ~WADSDriver();
    std::string to_str();
    
    int put(std::string key, unsigned int ver, std::string data_type, 
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(bool blocking, std::string key, unsigned int ver, std::string data_type,
            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    void handle_ri_msg(std::map<std::string, std::string> msg_map);
};

/***********************************  MWADSDriver : WADSDriver  ***********************************/
class MWADSDriver : public WADSDriver { // Markov
  public:
    MWADSDriver(int app_id, int base_client_id, int num_local_peers,
                std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
    : WADSDriver(app_id, base_client_id, num_local_peers, KV_DATA_ID,
                 lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport)
    {
      LOG(INFO) << "MWADSDriver:: constructed.";
    }
    
    MWADSDriver(int app_id, int base_client_id, int num_local_peers, MPI_Comm mpi_comm,
                std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
    : WADSDriver(app_id, base_client_id, num_local_peers, KV_DATA_ID, mpi_comm,
                 lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport)
    {
      LOG(INFO) << "MWADSDriver:: constructed.";
    }
    
    ~MWADSDriver() { LOG(INFO) << "MWADSDriver:: destructed."; }
};

/***********************************  SWADSDriver : WADSDriver  ***********************************/
class SWADSDriver : public WADSDriver { // Spatial
  public:
    SWADSDriver(int app_id, int base_client_id, int num_local_peers,
                std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
    : WADSDriver(app_id, base_client_id, num_local_peers, LUCOOR_DATA_ID,
                 lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport)
    {
      LOG(INFO) << "SWADSDriver:: constructed.";
    }
    
    SWADSDriver(int app_id, int base_client_id, int num_local_peers, MPI_Comm mpi_comm,
                std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
    : WADSDriver(app_id, base_client_id, num_local_peers, LUCOOR_DATA_ID, mpi_comm,
                 lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport)
    {
      LOG(INFO) << "SWADSDriver:: constructed.";
    }
    
    ~SWADSDriver() { LOG(INFO) << "SWADSDriver:: destructed."; }
};

#endif //end of _DATASPACES_WA_H_