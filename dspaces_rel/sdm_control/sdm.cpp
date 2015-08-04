#include "sdm_helper.h"

/********************************************   KVTable   *****************************************/
KVTable::KVTable() { LOG(INFO) << "KVTable:: constructed."; }

KVTable::~KVTable() { LOG(INFO) << "KVTable:: deconstructed."; }

int KVTable::get_key_ver(std::string key, unsigned int ver, 
                         int& p_id, std::string &data_type, char &ds_id, 
                         int &size, int &ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!kv_info_map.contains(kv) )
    return 1;
  
  key_ver_info& kv_info = *kv_info_map[kv];
  p_id = kv_info.p_id;
  data_type = kv_info.data_type;
  ds_id = kv_info.ds_id;
  
  if (size != -1) {
    size = kv_info.size;
    ndim = kv_info.ndim;
    gdim_ = kv_info.gdim_;
    lb_ = kv_info.lb_;
    ub_ = kv_info.ub_;
  }
  
  return 0;
}

int KVTable::put_key_ver(std::string key, unsigned int ver, 
                         int p_id, std::string data_type, char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (kv_info_map.contains(kv) )
    return update_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_);

  return add_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
}

int KVTable::add_key_ver(std::string key, unsigned int ver, 
                         int p_id, std::string data_type, char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  kv_info_map[kv] = boost::make_shared<key_ver_info>(p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
  kv_mark_map[kv] = false;
  // 
  LOG(INFO) << "add_key_ver:: added <key= " << key << ", ver= " << ver << "> : ds_id= " << ds_id;
  return 0;
}

int KVTable::update_key_ver(std::string key, unsigned int ver, 
                            int p_id, std::string data_type, char ds_id, int size, int ndim, 
                            uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  del_key_ver(key, ver);
  add_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
  // 
  LOG(INFO) << "update_key_ver:: updated <key= " << key << ", ver= " << ver << "> : ds_id= " << ds_id;
  return 0;
}

int KVTable::del_key_ver(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!kv_info_map.contains(kv) ) {
    LOG(ERROR) << "del_key:: non-existing kv= <" << key << ", " << ver << ">";
    return 1;
  }
  kv_info_map.del(kv);
  kv_mark_map.del(kv);
  // 
  LOG(INFO) << "del_key:: deleted kv= <" << key << ", " << ver << ">";
  return 0;
}

bool KVTable::is_feasible_to_get(std::string key, unsigned int ver, 
                                 int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!kv_info_map.contains(kv) )
    return false;
  
  key_ver_info& kv_info = *kv_info_map[kv];
  
  if (ndim != kv_info.ndim || size != kv_info.size)
    return false;
  
  for(int i = 0; i < ndim; i++) {
    if (gdim_[i] != kv_info.gdim_[i] )
      return false;
    if (lb_[i] < kv_info.lb_[i] )
      return false;
    if (ub_[i] > kv_info.ub_[i] )
      return false;
  }
  
  return true;
}

std::string KVTable::to_str()
{
  std::stringstream ss;
  
  for (std::map<key_ver_pair, boost::shared_ptr<key_ver_info> >::iterator it = kv_info_map.begin(); it != kv_info_map.end(); it++) {
    key_ver_pair kv = it->first;
    key_ver_info& kv_info = *(it->second);
    
    int ndim = kv_info.ndim;
    ss << "<key= " << kv.first << ", ver= " << boost::lexical_cast<std::string>(kv.second) << ">\n";
          "\t p_id= " << boost::lexical_cast<std::string>(kv_info.p_id) << "\n"
          "\t data_type= " << kv_info.data_type << "\n"
          "\t ds_id= " << kv_info.ds_id << "\n"
          "\t mark= " << kv_mark_map[kv] << "\n"
          "\t size= " << kv_info.size << "\n"
          "\t ndim= " << ndim << "\n"
          "\t gdim= " << patch_sfc::arr_to_str<>(ndim, kv_info.gdim_) << "\n"
          "\t lb= " << patch_sfc::arr_to_str<>(ndim, kv_info.lb_) << "\n"
          "\t ub= " << patch_sfc::arr_to_str<>(ndim, kv_info.ub_) << "\n";
  }
  
  return ss.str();
}

std::map<std::string, std::string> KVTable::to_str_str_map()
{
  std::map<std::string, std::string> str_str_map;
  int counter = 0;
  for (std::map<key_ver_pair, boost::shared_ptr<key_ver_info> >::iterator it = kv_info_map.begin(); it != kv_info_map.end(); it++) {
    key_ver_pair kv = it->first;
    key_ver_info& kv_info = *(it->second);
    
    uint64_t size = kv_info.size;

    std::string counter_str = boost::lexical_cast<std::string>(counter);
    str_str_map["key_" + counter_str] = kv.first;
    str_str_map["ver_" + counter_str] = boost::lexical_cast<std::string>(kv.second);
    str_str_map["p_id_" + counter_str] = boost::lexical_cast<std::string>(kv_info.p_id);
    str_str_map["data_type_" + counter_str] = kv_info.data_type;
    str_str_map["ds_id_" + counter_str] = kv_info.ds_id;
    str_str_map["size_" + counter_str] = boost::lexical_cast<std::string>(kv_info.size);
    str_str_map["ndim_" + counter_str] = boost::lexical_cast<std::string>(ndim);
    str_str_map["gdim_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.gdim_);
    str_str_map["lb_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.lb_);
    str_str_map["ub_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.ub_);
    
    counter++;
  }
  
  return str_str_map;
}

int KVTable::mark_all()
{
  for (std::map<key_ver_pair, bool>::iterator it = kv_mark_map.begin(); it != kv_mark_map.end(); it++)
    it->second = true;
  
  return 0;
}

std::map<std::string, std::string> KVTable::to_unmarked_str_str_map()
{
  std::map<std::string, std::string> str_str_map;
  int counter = 0;
  for (std::map<key_ver_pair, boost::shared_ptr<key_ver_info> >::iterator it = kv_info_map.begin(); it != kv_info_map.end(); it++) {
    key_ver_pair kv = it->first;
    if (!kv_mark_map[kv] ) {
      key_ver_info& kv_info = *(it->second);
      
      int ndim = kv_info.ndim;

      std::string counter_str = boost::lexical_cast<std::string>(counter);
      str_str_map["key_" + counter_str] = kv.first;
      str_str_map["ver_" + counter_str] = boost::lexical_cast<std::string>(kv.second);
      str_str_map["p_id_" + counter_str] = boost::lexical_cast<std::string>(kv_info.p_id);
      str_str_map["data_type_" + counter_str] = kv_info.data_type;
      str_str_map["ds_id_" + counter_str] = kv_info.ds_id;
      str_str_map["size_" + counter_str] = boost::lexical_cast<std::string>(kv_info.size);
      str_str_map["ndim_" + counter_str] = boost::lexical_cast<std::string>(ndim);
      str_str_map["gdim_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.gdim_);
      str_str_map["lb_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.lb_);
      str_str_map["ub_" + counter_str] = patch_sfc::arr_to_str<>(ndim, kv_info.ub_);
      
      counter++;
    }
  }
  
  return str_str_map;
}

/********************************************  MsgCoder  ******************************************/
MsgCoder::MsgCoder() { LOG(INFO) << "MsgCoder:: constructed."; }

MsgCoder::~MsgCoder() { LOG(INFO) << "MsgCoder:: destructed."; }

std::map<std::string, std::string> MsgCoder::decode(char* msg)
{
  // msg: serialized std::map<std::string, std::string>
  std::map<std::string, std::string> msg_map;
  
  std::stringstream ss;
  ss << msg;
  boost::archive::text_iarchive ia(ss);
  ia >> msg_map;
  
  return msg_map;
}

std::string MsgCoder::encode(std::map<std::string, std::string> msg_map)
{
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << msg_map;
  
  return ss.str();
}

int MsgCoder::decode_msg_map(std::map<std::string, std::string> msg_map,
                             int& ndim, COOR_T* &lcoor_, COOR_T* &ucoor_)
{
  try {
    ndim = boost::lexical_cast<int>(msg_map["ndim"] );
    
    boost::char_separator<char> sep(",");
    
    boost::tokenizer<boost::char_separator<char> > lcoor_tokens(msg_map["lcoor_"], sep);
    boost::tokenizer<boost::char_separator<char> > ucoor_tokens(msg_map["ucoor_"], sep);
    
    lcoor_ = (COOR_T*)malloc(ndim*sizeof(COOR_T) );
    ucoor_ = (COOR_T*)malloc(ndim*sizeof(COOR_T) );
    
    boost::tokenizer<boost::char_separator<char> >::iterator lcoor_it = lcoor_tokens.begin();
    boost::tokenizer<boost::char_separator<char> >::iterator ucoor_it = ucoor_tokens.begin();
    for (int i = 0; i < ndim; i++, lcoor_it++, ucoor_it++) {
      lcoor_[i] = boost::lexical_cast<COOR_T>(*lcoor_it);
      ucoor_[i] = boost::lexical_cast<COOR_T>(*ucoor_it);
    }
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "decode_msg_map:: Exception=" << ex.what();
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
  catch(std::exception& ex) {
    LOG(ERROR) << "decode_msg_map:: Exception=" << ex.what();
    return 1;
  }
  return 0;
}

int MsgCoder::encode_msg_map(std::map<std::string, std::string> &msg_map,
                             int ndim, COOR_T* lcoor_, COOR_T* ucoor_)
{
  try {
    msg_map["ndim"] = boost::lexical_cast<std::string>(ndim);
    msg_map["lcoor_"] = patch_sfc::arr_to_str<>(ndim, lcoor_);
    msg_map["ucoor_"] = patch_sfc::arr_to_str<>(ndim, ucoor_);
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "encode_msg_map:: Exception=" << ex.what();
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
    msg_map["gdim_"] = patch_sfc::arr_to_str<>(ndim, gdim_);
    msg_map["lb_"] = patch_sfc::arr_to_str<>(ndim, lb_);
    msg_map["ub_"] = patch_sfc::arr_to_str<>(ndim, ub_);
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "encode_msg_map:: Exception=" << ex.what();
    return 1;
  }
  return 0;
}
