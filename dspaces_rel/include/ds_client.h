#ifndef _DSCLIENT_H_
#define _DSCLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <map>
#include <unistd.h>

#include <string>

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
//for boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>

#include "ds_drive.h"
#include "dht_node.h"
#include "packet.h"

#include "ib_delivery.h"

#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) do { int r=x; if (r){ printf("error: " #x " failed (returned non-zero=%d).", r); } } while (0)
#define TEST_Z(x)  do { if (!(x)) printf("error: " #x " failed (returned zero or null)."); } while (0)
#endif
//********************************   thread_safe_map  **********************************//
template <typename Tk, typename Tv>
struct thread_safe_map
{
  private:
    boost::mutex mutex;
    typename std::map<Tk, Tv> map;
    typename std::map<Tk, Tv>::iterator map_it;
  public:
    thread_safe_map() {};
    ~thread_safe_map() {};
    
    Tv& operator[](Tk k) {
      boost::lock_guard<boost::mutex> guard(this->mutex);
      return map[k];
    };
    
    int del(Tk k)
    {
      map_it = map.find(k);
      map.erase(map_it);
    };
    
    bool contains(Tk k)
    {
      return !(map.count(k) == 0);
    };
    
    typename std::map<Tk, Tv>::iterator begin()
    {
      return map.begin();
    };
    
    typename std::map<Tk, Tv>::iterator end()
    {
      return map.end();
    };
};
//************************************   syncer  **********************************//
template <typename T>
struct syncer{
  private:
    boost::mutex mutex;
  
    typename std::map<T, boost::shared_ptr<boost::condition_variable> > point_cv_map;
    typename std::map<T, boost::shared_ptr<boost::condition_variable> >::iterator point_cv_map_it;
    typename std::map<T, boost::shared_ptr<boost::mutex> > point_m_map;
    typename std::map<T, boost::shared_ptr<boost::mutex> >::iterator point_m_map_it;
    typename std::map<T, int> point_numpeers_map;
    typename std::map<T, int>::iterator point_numpeers_map_it;
  public:
    syncer()
    {
      LOG(INFO) << "syncer:: constructed.";
    };
    ~syncer()
    {
      LOG(INFO) << "syncer:: destructed.";
    };
    int add_sync_point(T point, int num_peers)
    {
      boost::lock_guard<boost::mutex> guard(this->mutex);
      
      if (point_cv_map.count(point) ){
        LOG(ERROR) << "add_sync_point:: already added point.";
        return 1;
      }
      boost::shared_ptr<boost::condition_variable> t_cv_( new boost::condition_variable() );
      boost::shared_ptr<boost::mutex> t_m_( new boost::mutex() );
      
      point_cv_map[point] = t_cv_;
      point_m_map[point] = t_m_;
      point_numpeers_map[point] = num_peers;
      
      return 0;
    };
    int del_sync_point(T point)
    {
      boost::lock_guard<boost::mutex> guard(this->mutex);
      
      if (!point_cv_map.count(point) ){
        LOG(ERROR) << "del_sync_point:: non-existing point.";
        return 1;
      }
      point_cv_map_it = point_cv_map.find(point);
      point_cv_map.erase(point_cv_map_it);
      
      point_m_map_it = point_m_map.find(point);
      point_m_map.erase(point_m_map_it);
      
      point_numpeers_map_it = point_numpeers_map.find(point);
      point_numpeers_map.erase(point_numpeers_map_it);
      
      return 0;
    };
    int wait(T point)
    {
      boost::mutex::scoped_lock lock(*point_m_map[point]);
      point_cv_map[point]->wait(lock);
      
      return 0;
    };
    int notify(T point)
    {
      boost::lock_guard<boost::mutex> guard(this->mutex);
      
      int num_peers_to_wait = point_numpeers_map[point];
      --num_peers_to_wait;
      
      if (num_peers_to_wait == 0){
        point_cv_map[point]->notify_one();
        return 0;
      }
      point_numpeers_map[point] = num_peers_to_wait;
      
      return 0;
    };
};
/*******************************************************************************************/
class IMsgCoder
{
  public:
    IMsgCoder();
    ~IMsgCoder();
    std::map<std::string, std::string> decode(char* msg);
    std::string encode(std::map<std::string, std::string> msg_map);
    int decode_i_msg(std::map<std::string, std::string> msg_map, 
                     std::string& key, unsigned int& ver, std::string& data_type,
                     int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
};

//Server side of blocking communication channel over dataspaces
//Single server <-> many clients
//used by RIManager
typedef boost::function<void(char*)> function_cb_on_recv;

class BCServer
{
  public:
    BCServer(int app_id, int num_clients, int msg_size, 
             std::string base_comm_var_name, function_cb_on_recv f_cb,
             boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~BCServer();
    void init_listen_client(int client_id);
    void init_listen_all();
    void reinit_listen_client(int client_id);
  private:
    int app_id, num_clients, msg_size;
    std::string base_comm_var_name;
    function_cb_on_recv f_cb;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

//Client side of blocking communication channel over dataspaces
class BCClient
{
  public:
    BCClient(int app_id, int num_others, int max_msg_size, 
             std::string base_comm_var_name,
             boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~BCClient();
    int send(std::map<std::string, std::string> msg_map);
  private:
    int app_id, num_others, max_msg_size;
    std::string base_comm_var_name;
    std::string comm_var_name;
    IMsgCoder imsg_coder;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

//Remote Query Table
//A key 'k' can only be in a single dataspaces
typedef std::pair<std::string, unsigned int> key_ver_pair;
struct RQTable
{
  public:
    RQTable();
    ~RQTable();
    int get_key_ver(std::string key, unsigned int ver, 
                     std::string& data_type, char& ds_id, int* size, int* ndim, 
                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int put_from_map(std::map<std::string, std::string> map);
    int put_key_ver(std::string key, unsigned int ver, 
                    std::string data_type, char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int add_key_ver(std::string key, unsigned int ver, 
                    std::string data_type, char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int update_key_ver(std::string key, unsigned int ver, 
                       std::string data_type, char ds_id, int size, int ndim, 
                       uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int del_key_ver(std::string key, unsigned int ver);
    bool is_feasible_to_get(std::string key, unsigned int ver, 
                            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    std::string to_str();
    std::map<std::string, std::string> to_str_str_map();
  private:
    thread_safe_map<key_ver_pair, char> key_ver__dsid_map;
    thread_safe_map<key_ver_pair, std::string> key_ver__data_type_map;
    thread_safe_map<key_ver_pair, std::map<std::string, std::vector<uint64_t> > > key_ver__datainfo_map;
};

class RFPManager //Remote Fetch / Place Manager
{
  public:
    RFPManager(std::list<std::string> wa_ib_lport_list, boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~RFPManager();
    std::string get_ib_lport();
    
    bool receive_put(std::string ib_laddr, std::string ib_lport,
                     std::string key, unsigned int ver, std::string data_type,
                     int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    void handle_ib_receive(std::string key, unsigned int ver, size_t data_size, void* data_);
    
    bool get_send(std::string key, unsigned int ver, std::string data_type, 
                  int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                  const char* ib_laddr, const char* ib_lport);
    size_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
  private:
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<DDManager> dd_manager_;
    
    std::map<key_ver_pair, size_t> key_ver__recvedsize_map;
    std::map<key_ver_pair, void*> key_ver__data_map;
};

//Remote Interaction Manager
//TODO: a better way for syncing client and server of bccomm
#define WAIT_TIME_FOR_BCCLIENT_DSLOCK 100*1000

const size_t RI_MAX_MSG_SIZE = 1000;
const size_t LI_MAX_MSG_SIZE = 1000;

const std::string REMOTE_QUERY = "rq";
const std::string REMOTE_QUERY_REPLY = "rq_reply";
const std::string REMOTE_FETCH = "rf";
const std::string REMOTE_GET = "rg";
const std::string REMOTE_GET_REPLY = "rg_reply";
const std::string REMOTE_PUT = "rp";
const std::string REMOTE_PUT_REPLY = "rp_reply";
const std::string LOCAL_GET = "lg";
const std::string LOCAL_GET_REPLY = "lg_reply";
const std::string LOCAL_PUT = "lp";
const std::string LOCAL_PUT_REPLY = "lp_reply";

const std::string REMOTE_RQTABLE = "rrqt";
const std::string REMOTE_PLACE = "rp";
const std::string REMOTE_PLACE_REPLY = "rp_reply";

typedef std::pair<std::string, std::string> laddr_lport_pair;

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id, 
              char* lip, int lport, char* ipeer_lip, int ipeer_lport, 
              char* ib_laddr, std::list<std::string> wa_ib_lport_list);
    ~RIManager();
    std::string to_str();
    
    void handle_ri_req(char* ri_req);
    void handle_r_get(int app_id, std::map<std::string, std::string> r_get_map);
    
    void handle_li_req(char* li_req);
    void handle_l_put(std::map<std::string, std::string> l_put_map);
    
    void handle_wamsg(std::map<std::string, std::string> wamsg_map);
    void handle_r_query(std::map<std::string, std::string> r_query_map);
    void handle_rq_reply(std::map<std::string, std::string> rq_reply_map);
    void handle_r_fetch(std::map<std::string, std::string> r_fetch_map);
    void handle_r_rqtable(std::map<std::string, std::string> r_rqtable_map);
    void handle_r_place(std::map<std::string, std::string> r_place_map);
    void handle_rp_reply(std::map<std::string, std::string> rp_reply_map);
    void handle_r_subscribe(std::map<std::string, std::string> r_subs_map);
    
    int remote_subscribe(std::string key, unsigned int ver);
    int remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map);
    void handle_receive_put(std::map<std::string, std::string> str_str_map);
    int bcast_rq_table();
    int remote_query(std::string key, unsigned int ver);
    int broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map);
    int send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map);
    int remote_place(std::string key, unsigned int ver);
  private:
    //ImpRem: Since handle_ core functions are called by client threads properties must be thread-safe
    char id;
    int num_cnodes, app_id;
    char* ib_laddr;
    IMsgCoder imsg_coder;
    
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCServer> li_bc_server_;
    boost::shared_ptr<BCServer> ri_bc_server_;
    boost::shared_ptr<DHTNode> dht_node_;
    boost::shared_ptr<RFPManager> rfp_manager_;
    thread_safe_map<int, boost::shared_ptr<BCClient> > appid_bcclient_map; //TODO: prettify
    
    RQTable rq_table;
    syncer<key_ver_pair> rq_syncer;
    
    thread_safe_map<key_ver_pair, laddr_lport_pair> key_ver__laddr_lport_map;
    syncer<key_ver_pair> rp_syncer;
    
    thread_safe_map<key_ver_pair, char> key_ver__subsed_ds_id_map;
    
    syncer<key_ver_pair> rf_receive_put_syncer;
    syncer<key_ver_pair> rp_receive_put_syncer;
};

#endif //end of _DSCLIENT_H