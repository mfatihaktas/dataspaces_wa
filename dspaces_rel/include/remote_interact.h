#ifndef _REMOTE_INTERACT_H_
#define _REMOTE_INTERACT_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

#include "ds_client.h"
#include "sdm_control.h"
#include "profiler.h"
#include "trans.h"

/******************************************  RFPManager  ******************************************/
class RFPManager { // Remote Fetch & Place
  private:
    DATA_ID_T data_id_t;
    
    boost::shared_ptr<Trans> trans_;
    boost::shared_ptr<DSDriver> ds_driver_;
    
    patch::thread_safe_map<std::string, uint64_t> data_id__recved_size_map;
    patch::thread_safe_map<std::string, void*> data_id__data_map;
    patch::syncer<unsigned int> rfp_syncer;
  public:
    RFPManager(DATA_ID_T data_id_t, std::string trans_protocol,
               std::string ib_lip, std::list<std::string> ib_lport_list,
               std::string tcp_lip, int tcp_lport,
               std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir,
               boost::shared_ptr<DSDriver> ds_driver_);
    ~RFPManager();
    std::string to_str();
    
    std::string get_lip();
    std::string get_lport();
    std::string get_tmpfs_dir();
    uint64_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
    
    int wa_put(std::string lip, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
    int wa_get(std::string lip, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver,
               int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
    bool is_being_get(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_);
    int wait_for_get(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_);
    int notify_remote_get_done(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_);
    
    void handle_recv(std::string data_id, uint64_t data_size, void* data_);
};

/*******************************************  RIManager  ******************************************/
// App - RI
const std::string GET = "g";
const std::string GET_REPLY = "gr";
const std::string BLOCKING_GET = "bg";
const std::string BLOCKING_GET_REPLY = "bgr";
const std::string PUT = "p";
const std::string PUT_REPLY = "pr";
const std::string DEL = "d";
const std::string DEL_REPLY = "dr";
// RI - RI
const std::string RI_TINFO_QUERY = "ri_tiq";
const std::string RI_TINFO_QUERY_REPLY = "ri_tiqr";
const std::string RI_GRIDFTP_PUT = "ri_gp";

const int CL__RIMANAGER_MAX_MSG_SIZE = 1000;

class RIManager {
  struct trans_info {
    std::string lip, lport, tmpfs_dir;
    
    trans_info(std::string lip, std::string lport, std::string tmpfs_dir)
    : lip(lip), lport(lport), tmpfs_dir(tmpfs_dir)
    {}
  };
  
  struct data_info {
    std::string data_type;
    int size;
    COOR_T* gdim_;
    
    data_info(std::string data_type, int size, COOR_T* gdim_)
    : data_type(data_type), size(size), gdim_(gdim_)
    {}
  };
  
  protected:
    //ImpRem: Since handle_ core functions are called by client threads, properties must be thread-safe
    int cl_id, base_client_id, num_client;
    DATA_ID_T data_id_t;
    std::string data_trans_protocol;
    
    patch_sdm::MsgCoder msg_coder;
    
    boost::shared_ptr<SDMSlave> sdm_slave_;
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<SDMNode> lsdm_node_; // Replacing bc_server--client
    boost::shared_ptr<RFPManager> rfp_manager_;
    patch::thread_safe_map<int, boost::shared_ptr<trans_info> > ds_id__trans_info_map;
    patch::thread_safe_map<unsigned int, boost::shared_ptr<data_info> > data_id_hash__data_info_map;
    
    patch::syncer<unsigned int> ri_syncer;
    
    boost::asio::io_service io_service;
    boost::asio::signal_set signals;
  public:
    RIManager(int cl_id, int base_client_id, int num_client, DATA_ID_T data_id_t,
              std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
              std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
              std::string tcp_lip, int tcp_lport,
              std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir);
    ~RIManager();
    void close();
    virtual std::string to_str();
    
    void handle_app_msg(std::map<std::string, std::string> msg_map);
    void handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map);
    void handle_put(int p_id, std::map<std::string, std::string> put_map);
    // void handle_del(key_ver_pair kv);
    
    int trans_info_query(int to_id, std::map<std::string, std::string> msg_map);
    void handle_rimsg(std::map<std::string, std::string> rimsg_map);
    void handle_tinfo_query(std::map<std::string, std::string> msg_map);
    int remote_get(std::map<std::string, std::string> msg_map);
    void remote_put(std::map<std::string, std::string> msg_map);
    void handle_tinfo_query_reply(std::map<std::string, std::string> msg_map);
    void handle_gridftp_put(std::map<std::string, std::string> msg_map);
    
    void handle_dm_act(std::map<std::string, std::string> dm_act_map);
    void handle_dm_move(std::map<std::string, std::string> msg_map);
    void handle_dm_del(std::map<std::string, std::string> msg_map);
};

/***********************************  MSRIManager : RIManager  ************************************/
class MSRIManager : public RIManager { // Markov Slave
  public:
    MSRIManager(int cl_id, int base_client_id, int num_client,
                std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                int ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
                std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                std::string tcp_lip, int tcp_lport,
                std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
    : RIManager(cl_id, base_client_id, num_client, KV_DATA_ID,
                lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                data_trans_protocol, ib_lip, ib_lport_list,
                tcp_lip, tcp_lport,
                gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir)
    {
      sdm_slave_ = boost::make_shared<MSDMSlave>(ds_id, control_lip, control_lport, join_control_lip, join_control_lport,
                                                 boost::bind(&RIManager::handle_rimsg, this, _1), boost::bind(&RIManager::handle_dm_act, this, _1) );
      // Note: sdm_node within sdm_slave takes longer to connect especially for wide-area scenario which
      // makes the wait for join within sdm_node constructor useless. I had to move initialization of
      // lsdm_node here after sdm_slave is ready
      lsdm_node_ = boost::make_shared<SDMNode>(
                     "m", true,
                     boost::lexical_cast<int>(cl_id), lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                     boost::bind(&RIManager::handle_app_msg, this, _1) );
      // 
      log_(INFO, "constructed; \n" << to_str() )
      
      signals.async_wait(boost::bind(&RIManager::close, this) );
      io_service.run();
    }
    ~MSRIManager() { log_(INFO, "destructed.") }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "\t sdm_slave= \n" << sdm_slave_->to_str() << "\n"
         << "\t lsdm_node= \n" << lsdm_node_->to_str() << "\n";
      return ss.str();
    }
};

/***********************************  SSRIManager : RIManager  ************************************/
class SSRIManager : public RIManager { // Spatial Slave
  public:
    SSRIManager(int cl_id, int base_client_id, int num_client,
                std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                int ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
                std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                std::string tcp_lip, int tcp_lport,
                std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
    : RIManager(cl_id, base_client_id, num_client, LUCOOR_DATA_ID,
                lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                data_trans_protocol, ib_lip, ib_lport_list,
                tcp_lip, tcp_lport,
                gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir)
    {
      sdm_slave_ = boost::make_shared<SSDMSlave>(ds_id, control_lip, control_lport, join_control_lip, join_control_lport,
                                                 boost::bind(&RIManager::handle_rimsg, this, _1), boost::bind(&RIManager::handle_dm_act, this, _1) );
      lsdm_node_ = boost::make_shared<SDMNode>(
                     "m", true,
                     boost::lexical_cast<int>(cl_id), lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                     boost::bind(&RIManager::handle_app_msg, this, _1) );
      // 
      log_(INFO, "constructed; \n" << to_str() )
      
      signals.async_wait(boost::bind(&RIManager::close, this) );
      io_service.run();
    }
    ~SSRIManager() { log_(INFO, "destructed.") }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "\t sdm_slave= \n" << sdm_slave_->to_str() << "\n"
         << "\t lsdm_node= \n" << lsdm_node_->to_str() << "\n";
      return ss.str();
    }
};

/***********************************  MMRIManager : RIManager  ************************************/
class MMRIManager : public RIManager { // Markov Master
  public:
    MMRIManager(int cl_id, int base_client_id, int num_client,
                std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                int ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
                MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch,
                std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                std::string tcp_lip, int tcp_lport,
                std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
    : RIManager(cl_id, base_client_id, num_client, KV_DATA_ID,
                lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                data_trans_protocol, ib_lip, ib_lport_list,
                tcp_lip, tcp_lport,
                gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir)
    {
      boost::shared_ptr<SDMSlave> t_sdm_slave_(
        new MSDMMaster(ds_id, control_lip, control_lport, join_control_lip, join_control_lport,
                       boost::bind(&RIManager::handle_rimsg, this, _1), boost::bind(&RIManager::handle_dm_act, this, _1),
                       malgo_t, max_num_key_ver_in_mpbuffer, w_prefetch) );
      sdm_slave_ = t_sdm_slave_;
      
      lsdm_node_ = boost::make_shared<SDMNode>(
                     "m", true,
                     boost::lexical_cast<int>(cl_id), lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                     boost::bind(&RIManager::handle_app_msg, this, _1) );
      // 
      log_(INFO, "constructed; \n" << to_str() )
      
      signals.async_wait(boost::bind(&RIManager::close, this) );
      io_service.run();
    }
    ~MMRIManager() { log_(INFO, "destructed.") }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "\t sdm_slave= \n" << sdm_slave_->to_str() << "\n"
         << "\t lsdm_node= \n" << lsdm_node_->to_str() << "\n";
      return ss.str();
    }
};

/***********************************  SMRIManager : RIManager  ************************************/
class SMRIManager : public RIManager { // Spatial Master
  public:
    SMRIManager(int cl_id, int base_client_id, int num_client,
                std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                int ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
                SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch,
                std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                std::string tcp_lip, int tcp_lport,
                std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
    : RIManager(cl_id, base_client_id, num_client, LUCOOR_DATA_ID,
                lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                data_trans_protocol, ib_lip, ib_lport_list,
                tcp_lip, tcp_lport,
                gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir)
    {
      boost::shared_ptr<SDMSlave> t_sdm_slave_(
        new SSDMMaster(ds_id, control_lip, control_lport, join_control_lip, join_control_lport,
                       boost::bind(&RIManager::handle_rimsg, this, _1), boost::bind(&RIManager::handle_dm_act, this, _1),
                       salgo_t, lcoor_, ucoor_, sexpand_length, w_prefetch) );
      sdm_slave_ = t_sdm_slave_;
      
      lsdm_node_ = boost::make_shared<SDMNode>(
                     "m", true,
                     boost::lexical_cast<int>(cl_id), lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
                     boost::bind(&RIManager::handle_app_msg, this, _1) );
      // 
      log_(INFO, "constructed; \n" << to_str() )
      
      signals.async_wait(boost::bind(&RIManager::close, this) );
      io_service.run();
    }
    ~SMRIManager() { log_(INFO, "destructed.") }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "\t sdm_slave= \n" << sdm_slave_->to_str() << "\n"
         << "\t lsdm_node= \n" << lsdm_node_->to_str() << "\n";
      return ss.str();
    }
};

#endif //end of _REMOTE_INTERACT_H_