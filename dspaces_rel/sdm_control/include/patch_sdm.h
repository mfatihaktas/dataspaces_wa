#ifndef _PATCH_SDM_H_
#define _PATCH_SDM_H_

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>

#include "patch.h"
#include "sfc.h"

typedef char DATA_ID_T;
const DATA_ID_T KV_DATA_ID = 'k';
const DATA_ID_T LUCOOR_DATA_ID = 'l';

namespace patch_sdm {
  std::string get_data_id(DATA_ID_T data_id_t, std::string key, unsigned int ver, COOR_T* lb_, COOR_T* ub_);
  std::string get_data_id(DATA_ID_T data_id_t, std::map<std::string, std::string> msg_map);
  
  #define HASH_PRIME 54059
  #define HASH_PRIME_2 76963
  #define HASH_PRIME_3 86969
  unsigned int hash_str(std::string str);
  
  /********************************************  MsgCoder  ******************************************/
  class MsgCoder {
    public:
      MsgCoder();
      ~MsgCoder();
      
      int decode(char* msg, std::map<std::string, std::string>& msg_map);
      int encode(std::map<std::string, std::string> msg_map, std::string& str);
      
      int decode_msg_map(std::map<std::string, std::string> msg_map,
                         int &ndim, std::string& key, unsigned int& ver, COOR_T* &lb_, COOR_T* &ub_);
      
      int decode_msg_map(std::map<std::string, std::string> msg_map,
                         std::string& key, unsigned int& ver, std::string& data_type,
                         int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
      
      int encode_msg_map(std::map<std::string, std::string> &msg_map,
                         int ndim, std::string key, unsigned int ver, COOR_T* lb_, COOR_T* ub_);
      
      int encode_msg_map(std::map<std::string, std::string> &msg_map, 
                         std::string key, unsigned int ver, std::string data_type,
                         int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
  };
}

#endif //_PATCH_SDM_H_