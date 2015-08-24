#ifndef _SDM_H_
#define _SDM_H_

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "patch_sdm.h"
#include "sfc.h"

const char KV_DATA_ID = 'k';
const char LUCOOR_DATA_ID = 'l';
std::string get_data_id(char data_id_t, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);

/********************************************   KVTable   *****************************************/
// typedef std::pair<std::string, unsigned int> key_ver_pair;
// struct KVTable { // Key Ver
//   struct key_ver_info {
//     public:
//       int p_id;
//       std::string data_type;
//       char ds_id;
//       int size, ndim;
//       uint64_t *gdim_, *lb_, *ub_;
      
//       key_ver_info(int p_id, std::string data_type, char ds_id,
//                   int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
//       : p_id(p_id), data_type(data_type), ds_id(ds_id),
//         size(size), ndim(ndim), gdim_(gdim_), lb_(lb_), ub_(ub_) {}
      
//       ~key_ver_info() { free(gdim_); free(lb_); free(ub_); }
//   };
//   private:
//     patch_sdm::thread_safe_map<key_ver_pair, boost::shared_ptr<key_ver_info> > kv_info_map;
//     patch_sdm::thread_safe_map<key_ver_pair, bool> kv_mark_map;
//   public:
//     KVTable();
//     ~KVTable();
//     int get_key_ver(std::string key, unsigned int ver,
//                     int& p_id, std::string &data_type, char &ds_id,
//                     int &size, int &ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
//     int put_from_map(std::map<std::string, std::string> map);
//     int put_key_ver(std::string key, unsigned int ver,
//                     int p_id, std::string data_type, char ds_id, int size, int ndim, 
//                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
//     int add_key_ver(std::string key, unsigned int ver,
//                     int p_id, std::string data_type, char ds_id, int size, int ndim, 
//                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
//     int update_key_ver(std::string key, unsigned int ver,
//                       int p_id, std::string data_type, char ds_id, int size, int ndim, 
//                       uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
//     int del_key_ver(std::string key, unsigned int ver);
//     bool is_feasible_to_get(std::string key, unsigned int ver,
//                             int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
//     std::string to_str();
//     std::map<std::string, std::string> to_str_str_map();
    
//     int mark_all();
//     std::map<std::string, std::string> to_unmarked_str_str_map();
// };

/********************************************  MsgCoder  ******************************************/
class MsgCoder {
  public:
    MsgCoder();
    ~MsgCoder();
    std::map<std::string, std::string> decode(char* msg);
    std::string encode(std::map<std::string, std::string> msg_map);
    
    int decode_msg_map(std::map<std::string, std::string> msg_map,
                       int &ndim, std::string& key, unsigned int& ver, COOR_T* &lcoor_, COOR_T* &ucoor_);
    
    int decode_msg_map(std::map<std::string, std::string> msg_map,
                       std::string& key, unsigned int& ver, std::string& data_type,
                       int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
    
    int encode_msg_map(std::map<std::string, std::string> &msg_map,
                       int ndim, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    int encode_msg_map(std::map<std::string, std::string> &msg_map, 
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
};

#endif //_SDM_H_
