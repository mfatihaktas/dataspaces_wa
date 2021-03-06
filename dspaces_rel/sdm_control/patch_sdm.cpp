#include "patch_sdm.h"

namespace patch_sdm {
  std::string get_data_id(char data_id_t, std::string key, unsigned int ver, COOR_T* lb_, COOR_T* ub_)
  {
    if (data_id_t == KV_DATA_ID)
      return key + "_" + boost::lexical_cast<std::string>(ver);
    else if (data_id_t == LUCOOR_DATA_ID)
      return patch::arr_to_str<>(NDIM, lb_) + "_" + patch::arr_to_str<>(NDIM, ub_);
    else
      log_(ERROR, "unknown data_id_t= " << data_id_t)
  }
  
  std::string get_data_id(DATA_ID_T data_id_t, std::map<std::string, std::string> msg_map)
  {
    if (data_id_t == KV_DATA_ID)
      return msg_map["key"] + "_" + msg_map["ver"];
    else if (data_id_t == LUCOOR_DATA_ID)
      return msg_map["lb_"] + "_" + msg_map["ub_"];
    else
      log_(ERROR, "unknown data_id_t= " << data_id_t)
  }
  
  unsigned int hash_str(std::string str)
  {
    unsigned int h = 31; // Also prime
    const char* s_ = str.c_str();
    while (*s_) {
      h = (h * HASH_PRIME) ^ (s_[0] * HASH_PRIME_2);
      s_++;
    }
    return h; // return h % HASH_PRIME_3;
  }
  
  // unsigned int hash_str(std::string str)
  // {
  //   unsigned int seed = 0;
  //   const char* char_ = str.c_str();
  //   unsigned int hash = seed;
  //   while (*char_)
  //     hash = hash * 101  +  *char_++;
    
  //   return hash;
  // }
  
  /********************************************  MsgCoder  ******************************************/
  MsgCoder::MsgCoder() { /* LOG(INFO) << "MsgCoder:: constructed."; */ }
  MsgCoder::~MsgCoder() { /* LOG(INFO) << "MsgCoder:: destructed."; */ }
  
  // msg: serialized std::map<std::string, std::string>
  int MsgCoder::decode(char* msg, std::map<std::string, std::string>& msg_map)
  {
    try {
      std::stringstream ss;
      ss << msg;
      boost::archive::text_iarchive ia(ss);
      ia >> msg_map;
    }
    catch(boost::archive::archive_exception &e) {
      log_(ERROR, "exceptioon e= " << e.what() )
      return 1;
    }
    
    return 0;
  }
  
  int MsgCoder::encode(std::map<std::string, std::string> msg_map, std::string& str)
  {
    try {
      std::stringstream ss;
      boost::archive::text_oarchive oa(ss);
      oa << msg_map;
      
      str = ss.str();
    }
    catch(boost::archive::archive_exception &e) {
      log_(ERROR, "exceptioon e= " << e.what() )
      return 1;
    }
    
    return 0;
  }
  
  int MsgCoder::decode_msg_map(std::map<std::string, std::string> msg_map,
                               int& ndim, std::string& key, unsigned int& ver, COOR_T* &lb_, COOR_T* &ub_)
  {
    try {
      ndim = boost::lexical_cast<int>(msg_map["ndim"] );
      key = msg_map["key"];
      ver = boost::lexical_cast<unsigned int>(msg_map["ver"] );
      
      boost::char_separator<char> sep(",");
      
      boost::tokenizer<boost::char_separator<char> > lb_tokens(msg_map["lb_"], sep);
      boost::tokenizer<boost::char_separator<char> > ub_tokens(msg_map["ub_"], sep);
      
      lb_ = (COOR_T*)malloc(ndim*sizeof(COOR_T) );
      ub_ = (COOR_T*)malloc(ndim*sizeof(COOR_T) );
      
      boost::tokenizer<boost::char_separator<char> >::iterator lb_it = lb_tokens.begin();
      boost::tokenizer<boost::char_separator<char> >::iterator ub_it = ub_tokens.begin();
      for (int i = 0; i < ndim; i++, lb_it++, ub_it++) {
        lb_[i] = boost::lexical_cast<COOR_T>(*lb_it);
        ub_[i] = boost::lexical_cast<COOR_T>(*ub_it);
      }
    }
    catch (std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
      return 1;
    }
    return 0;
  }
  
  int MsgCoder::decode_msg_map(std::map<std::string, std::string> msg_map,
                               std::string& key, unsigned int& ver, std::string& data_type,
                               int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
  {
    try {
      key = msg_map["key"];
      ver = boost::lexical_cast<unsigned int>(msg_map["ver"] );
      data_type = msg_map["data_type"];
      size = boost::lexical_cast<int>(msg_map["size"] );
      ndim = boost::lexical_cast<int>(msg_map["ndim"] );
      
      boost::char_separator<char> sep(",");
      
      boost::tokenizer<boost::char_separator<char> > gdim_tokens(msg_map["gdim_"], sep);
      boost::tokenizer<boost::char_separator<char> > lb_tokens(msg_map["lb_"], sep);
      boost::tokenizer<boost::char_separator<char> > ub_tokens(msg_map["ub_"], sep);
      
      gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
      lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
      ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  
      boost::tokenizer<boost::char_separator<char> >::iterator gdim_it = gdim_tokens.begin();
      boost::tokenizer<boost::char_separator<char> >::iterator lb_it = lb_tokens.begin();
      boost::tokenizer<boost::char_separator<char> >::iterator ub_it = ub_tokens.begin();
      for (int i = 0; i < ndim; i++, gdim_it++, lb_it++, ub_it++) {
        gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_it);
        lb_[i] = boost::lexical_cast<uint64_t>(*lb_it);
        ub_[i] = boost::lexical_cast<uint64_t>(*ub_it);
      }
    }
    catch (std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
      return 1;
    }
    return 0;
  }
  
  int MsgCoder::encode_msg_map(std::map<std::string, std::string> &msg_map,
                               int ndim, std::string key, unsigned int ver, COOR_T* lb_, COOR_T* ub_)
  {
    try {
      msg_map["ndim"] = boost::lexical_cast<std::string>(ndim);
      msg_map["key"] = key;
      msg_map["ver"] = boost::lexical_cast<std::string>(ver);
      msg_map["lb_"] = patch::arr_to_str<>(ndim, lb_);
      msg_map["ub_"] = patch::arr_to_str<>(ndim, ub_);
    }
    catch (std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
      return 1;
    }
    return 0;
  }
  
  int MsgCoder::encode_msg_map(std::map<std::string, std::string> &msg_map, 
                               std::string key, unsigned int ver, std::string data_type,
                               int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
  {
    try {
      msg_map["key"] = key;
      msg_map["ver"] = boost::lexical_cast<std::string>(ver);
      msg_map["data_type"] = data_type;
      msg_map["size"] = boost::lexical_cast<std::string>(size);
      msg_map["ndim"] = boost::lexical_cast<std::string>(ndim);
      msg_map["gdim_"] = patch::arr_to_str<>(ndim, gdim_);
      msg_map["lb_"] = patch::arr_to_str<>(ndim, lb_);
      msg_map["ub_"] = patch::arr_to_str<>(ndim, ub_);
    }
    catch (std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
      return 1;
    }
    return 0;
  }
}
