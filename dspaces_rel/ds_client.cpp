#include "ds_client.h"

#ifndef _PRINT_FUNCS_
#define _PRINT_FUNCS_
void debug_print(std::string key, unsigned int ver, int size, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data, size_t data_length)
{
  LOG(INFO) << "debug_print::";
  std::cout << "key= " << key << "\n"
            << "ver= " << ver << "\n"
            << "size= " << size << "\n"
            << "ndim= " << ndim << "\n";
  std::cout << "gdim=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << gdim[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "lb=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << lb[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "ub=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << ub[i] << ", ";
  }
  std::cout << "\n";
  
  //
  if (data == NULL) {
    return;
  }
  
  std::cout << "data=";
  for (int i = 0; i < data_length; i++) {
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}

void print_str_map(std::map<std::string, std::string> str_map)
{
  for (std::map<std::string, std::string>::const_iterator it = str_map.begin(); 
       it != str_map.end(); it++){
    std::cout << "\t" << it->first << ": " << it->second << "\n";
  }
}
#endif

//***********************************  IMsgCoder  ********************************//
IMsgCoder::IMsgCoder()
{
  //
  LOG(INFO) << "IMsgCoder:: constructed.";
}

IMsgCoder::~IMsgCoder()
{
  //
  LOG(INFO) << "IMsgCoder:: destructed.";
}

std::map<std::string, std::string> IMsgCoder::decode(char* msg)
{
  //msg: serialized std::map<std::string, std::string>
  std::map<std::string, std::string> msg_map;
  
  std::stringstream ss;
  ss << msg;
  boost::archive::text_iarchive ia(ss);
  ia >> msg_map;
  //
  //LOG(INFO) << "decode:: msg_map= ";
  //print_str_map(msg_map);
  return msg_map;
}

int IMsgCoder::decode_msg_map(std::map<std::string, std::string> msg_map,
                              std::string& key, unsigned int& ver, std::string& data_type,
                              int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  try
  {
    key = msg_map["key"];
    data_type = msg_map["data_type"];
    ver = boost::lexical_cast<unsigned int>(msg_map["ver"]);
    size = boost::lexical_cast<int>(msg_map["size"]);
    ndim = boost::lexical_cast<int>(msg_map["ndim"]);
    
    boost::char_separator<char> sep(",");
    
    boost::tokenizer<boost::char_separator<char> > gdim_tokens(msg_map["gdim"], sep);
    boost::tokenizer<boost::char_separator<char> > lb_tokens(msg_map["lb"], sep);
    boost::tokenizer<boost::char_separator<char> > ub_tokens(msg_map["ub"], sep);
    
    uint64_t *t_gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    uint64_t *t_lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    uint64_t *t_ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    int i;
    boost::tokenizer<boost::char_separator<char> >::iterator gdim_it = gdim_tokens.begin();
    boost::tokenizer<boost::char_separator<char> >::iterator lb_it = lb_tokens.begin();
    boost::tokenizer<boost::char_separator<char> >::iterator ub_it = ub_tokens.begin();
    for (int i=0; i<ndim; i++)
    {
      t_gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_it);
      t_lb_[i] = boost::lexical_cast<uint64_t>(*lb_it);
      t_ub_[i] = boost::lexical_cast<uint64_t>(*ub_it);
      
      gdim_it++;
      lb_it++;
      ub_it++;
    }
    gdim_ = t_gdim_;
    lb_ = t_lb_;
    ub_ = t_ub_;
  }
  catch(std::exception & ex)
  {
    LOG(ERROR) << "decode_msg_map:: Exception=" << ex.what();
    return 1;
  }
  return 0;
}

int IMsgCoder::encode_msg_map(std::map<std::string, std::string> &msg_map, 
                              std::string key, unsigned int ver, std::string data_type,
                              int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  try
  {
    msg_map["key"] = key;
    msg_map["ver"] = boost::lexical_cast<std::string>(ver);
    msg_map["data_type"] = data_type;
    msg_map["size"] = boost::lexical_cast<std::string>(size);
    msg_map["ndim"] = boost::lexical_cast<std::string>(ndim);
    
    std::string gdim = "";
    std::string lb = "";
    std::string ub = "";
    for(int i = 0; i < ndim; i++){
      // LOG(INFO) << "encode_msg_map:: gdim_[" << i << "]= " << gdim_[i];
      // LOG(INFO) << "encode_msg_map:: lb_[" << i << "]= " << lb_[i];
      // LOG(INFO) << "encode_msg_map:: ub_[" << i << "]= " << ub_[i];
      gdim += boost::lexical_cast<std::string>(gdim_[i]);
      lb += boost::lexical_cast<std::string>(lb_[i]);
      ub += boost::lexical_cast<std::string>(ub_[i]);
      if (i < ndim-1){
        gdim += ",";
        lb += ",";
        ub += ",";
      }
    }
    msg_map["gdim"] = gdim;
    msg_map["lb"] = lb;
    msg_map["ub"] = ub;
  }
  catch(std::exception & ex)
  {
    LOG(ERROR) << "encode_msg_map:: Exception=" << ex.what();
    return 1;
  }
  return 0;
}

std::string IMsgCoder::encode(std::map<std::string, std::string> msg_map)
{
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << msg_map;
  
  return ss.str();
}
//************************************  BCServer  *********************************//
BCServer::BCServer(int app_id, int num_clients, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSpacesDriver> ds_driver_)
: app_id(app_id),
  num_clients(num_clients),
  msg_size(msg_size),
  base_comm_var_name(base_comm_var_name),
  f_cb(f_cb),
  ds_driver_ ( ds_driver_ )
{
  // 
  LOG(INFO) << "BCServer:: constructed for comm_var_name= " << base_comm_var_name;
}

BCServer::~BCServer()
{
  //ds_driver_->finalize();
  // 
  LOG(INFO) << "BCServer:: destructed.";
}

void BCServer::init_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  // 
  ds_driver_->reg_cb_on_get(key, f_cb);
  ds_driver_->init_riget_thread(key, msg_size);
  // 
  LOG(INFO) << "init_listen_client:: done for client_id= " << client_id;
}

void BCServer::init_listen_all()
{
  //Assume app_id of each client app is ordered as 1,2,...,num_clients
  for(int i=1; i <= num_clients; i++) {
    init_listen_client(i);
  }
  // 
  LOG(INFO) << "init_listen_all:: done.";
}

void BCServer::reinit_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->init_riget_thread(key, msg_size);
  //
  LOG(INFO) << "reinit_listen_client:: done for client_id= " << client_id;
}

//************************************  BCClient   ********************************//
BCClient::BCClient(int app_id, int num_others, int max_msg_size,
                   std::string base_comm_var_name, 
                   boost::shared_ptr<DSpacesDriver> ds_driver_)
: app_id(app_id),
  num_others(num_others),
  max_msg_size(max_msg_size),
  base_comm_var_name(base_comm_var_name),
  ds_driver_ ( ds_driver_ )
{
  comm_var_name = base_comm_var_name + boost::lexical_cast<std::string>(app_id);
  //ds_driver_->lock_on_write(comm_var_name.c_str() );
  //
  LOG(INFO) << "BCClient:: constructed for comm_var_name= " << comm_var_name;
}

BCClient::~BCClient()
{
  //ds_driver_->finalize();
  //
  LOG(INFO) << "BCClient:: destructed.";
}

int BCClient::send(std::map<std::string, std::string> msg_map)
{
  std::string msg_str = imsg_coder.encode(msg_map);
  
  int msg_size = msg_str.size();
  if (msg_size > max_msg_size){
    LOG(ERROR) << "send:: msg_size= " << msg_size << " > max_msg_size= " << max_msg_size;
    return 1;
  }
  
  //1 dimensional char array
  // uint64_t gdim = 0; //max_msg_size;
  // uint64_t lb = 0;
  // uint64_t ub = 0; //max_msg_size-1;
  uint64_t* gdim_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* lb_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* ub_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  for (int i = 0; i < 3; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  
  char *data_ = (char*)malloc(max_msg_size*sizeof(char) );
  strcpy(data_, msg_str.c_str() );
  for (int i = msg_size; i < max_msg_size - msg_size; i++) {
    data_[i] = '\0';
  }
  // sync_put(const char* var_name, unsigned int ver, int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
  int result = ds_driver_->sync_put(comm_var_name.c_str(), 0, max_msg_size*sizeof(char), 3, gdim_, lb_, ub_, data_);
  //int result = ds_driver_->sync_put_without_lock(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  free(data_);
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  //ds_driver_->lock_on_write(comm_var_name.c_str() );
  
  return result;
}
//************************************   RQTable   ********************************//
RQTable::RQTable()
{
  //
  LOG(INFO) << "RQTable:: constructed.";
}

RQTable::~RQTable()
{
  //
  LOG(INFO) << "RQTable:: deconstructed.";
}

int RQTable::get_key_ver(std::string key, unsigned int ver, 
                         std::string &data_type, char &ds_id, int &size, int &ndim, 
                         uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (!key_ver__dsid_map.contains(kv) ) {
    return 1;
  }
  ds_id = key_ver__dsid_map[kv];
  data_type = key_ver__data_type_map[kv];
  
  if (size != -1)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
    size = (int)datainfo_map["size"].back();
    ndim = (int)datainfo_map["ndim"].back();
    
    gdim_ = (uint64_t*)malloc(sizeof(uint64_t)*ndim );
    lb_ = (uint64_t*)malloc(sizeof(uint64_t)*ndim );
    ub_ = (uint64_t*)malloc(sizeof(uint64_t)*ndim );
    for (int i = 0; i < ndim; i++) {
      gdim_[i] = datainfo_map["gdim"][i];
      lb_[i] = datainfo_map["lb"][i];
      ub_[i] = datainfo_map["ub"][i];
    }
  }
  
  return 0;
}

int RQTable::put_key_ver(std::string key, unsigned int ver, 
                         std::string data_type, char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (key_ver__dsid_map.contains(kv) ){
    return update_key_ver(key, ver, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
  }
  return add_key_ver(key, ver, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
}

int RQTable::add_key_ver(std::string key, unsigned int ver, 
                         std::string data_type, char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  key_ver__dsid_map[kv] = ds_id;
  key_ver__data_type_map[kv] = data_type;
  key_ver__mark[kv] = false;
  
  if (size != 0)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map;
    
    std::vector<uint64_t> size_v;
    size_v.push_back((uint64_t)size);
    datainfo_map["size"] = size_v;
    
    std::vector<uint64_t> ndim_v;
    ndim_v.push_back((uint64_t)ndim);
    datainfo_map["ndim"] = ndim_v;
    
    std::vector<uint64_t> gdim_v;
    std::vector<uint64_t> lb_v;
    std::vector<uint64_t> ub_v;
    for(int i=0; i<ndim; i++){
      gdim_v.push_back(gdim_[i]);
      lb_v.push_back(lb_[i]);
      ub_v.push_back(ub_[i]);
    }
    datainfo_map["gdim"] = gdim_v;
    datainfo_map["lb"] = lb_v;
    datainfo_map["ub"] = ub_v;
    //
    key_ver__datainfo_map[kv] = datainfo_map;
  }
  //
  LOG(INFO) << "add_key_ver:: added <key= " << key << ", ver= " << ver << "> : ds_id= " << ds_id;
  return 0;
}

int RQTable::update_key_ver(std::string key, unsigned int ver, 
                            std::string data_type, char ds_id, int size, int ndim, 
                            uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  del_key_ver(key, ver);
  add_key_ver(key, ver, data_type, ds_id, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "update_key_ver:: updated <key= " << key << ", ver= " << ver << "> : ds_id= " << ds_id;
  return 0;
}

int RQTable::del_key_ver(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (!key_ver__dsid_map.contains(kv) ) {
    LOG(ERROR) << "del_key:: non-existing kv= <" << key << ", " << ver << ">";
    return 1;
  }
  key_ver__dsid_map.del(kv);
  key_ver__data_type_map.del(kv);
  key_ver__datainfo_map.del(kv);
  key_ver__mark.del(kv);
  //
  LOG(INFO) << "del_key:: deleted kv= <" << key << ", " << ver << ">";
  return 0;
}

bool RQTable::is_feasible_to_get(std::string key, unsigned int ver, 
                                 int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (!key_ver__datainfo_map.contains(kv) ) {
    return false;
  }
  
  std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
  int _ndim = (int)datainfo_map["ndim"].back();
  int _size = (int)datainfo_map["size"].back();
  if (ndim != _ndim || size != _size) {
    return false;
  }

  std::vector<uint64_t> _gdim_ = datainfo_map["gdim"];
  std::vector<uint64_t> _lb_ = datainfo_map["lb"];
  std::vector<uint64_t> _ub_ = datainfo_map["ub"];
  
  for(int i = 0; i < ndim; i++) {
    if (gdim_[i] != _gdim_[i]) {
      return false;
    }
    if (lb_[i] < _lb_[i]) {
      return false;
    }
    if (ub_[i] > _ub_[i]) {
      return false;
    }
  }
  
  return true;
}

std::string RQTable::to_str()
{
  std::stringstream ss;
  
  for (std::map<key_ver_pair, char>::iterator it=key_ver__dsid_map.begin(); it!=key_ver__dsid_map.end(); it++){
    key_ver_pair kv = it->first;
    
    ss << "<key= " << kv.first << ", ver= " << kv.second << ">\n";
    ss << "\tds_id=" << it->second << "\n";

    ss << "\tdata_type=" << key_ver__data_type_map[kv] << "\n";
    ss << "\tmark=" << key_ver__mark[kv] << "\n";

    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
    ss << "\tsize=" << datainfo_map["size"].back() << "\n";
    int ndim = (int)datainfo_map["ndim"].back();
    std::string gdim = "";
    std::string lb = "";
    std::string ub = "";
    for(int i=0; i<ndim; i++){
      gdim += boost::lexical_cast<std::string>(datainfo_map["gdim"][i]);
      lb += boost::lexical_cast<std::string>(datainfo_map["lb"][i]);
      ub += boost::lexical_cast<std::string>(datainfo_map["ub"][i]);
      if (i < ndim-1){
        gdim += ",";
        lb += ",";
        ub += ",";
      }
    }
    ss << "\tndim=" << ndim << "\n";
    ss << "\tgdim=" << gdim << "\n";
    ss << "\tlb=" << lb << "\n";
    ss << "\tub=" << ub << "\n";
    ss << "\n";
  }
  
  return ss.str();
}

std::map<std::string, std::string> RQTable::to_str_str_map()
{
  std::map<std::string, std::string> str_str_map;
  
  int counter = 0;
  for (std::map<key_ver_pair, char>::iterator it = key_ver__dsid_map.begin(); it != key_ver__dsid_map.end(); it++) 
  {
    key_ver_pair kv = it->first;
    std::string key = kv.first;
    unsigned int ver = kv.second;
    char ds_id = it->second;
    
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
    uint64_t size = datainfo_map["size"].back();
    int ndim = (int)datainfo_map["ndim"].back();
    std::string gdim = "";
    std::string lb = "";
    std::string ub = "";
    for(int i = 0; i < ndim; i++) {
      gdim += boost::lexical_cast<std::string>(datainfo_map["gdim"][i]);
      lb += boost::lexical_cast<std::string>(datainfo_map["lb"][i]);
      ub += boost::lexical_cast<std::string>(datainfo_map["ub"][i]);
      if (i < ndim-1) {
        gdim += ",";
        lb += ",";
        ub += ",";
      }
    }
    //
    std::string counter_str = boost::lexical_cast<std::string>(counter);
    str_str_map["key_" + counter_str] = key;
    str_str_map["ver_" + counter_str] = boost::lexical_cast<std::string>(ver);
    str_str_map["ds_id_" + counter_str] = ds_id;
    str_str_map["size_" + counter_str] = boost::lexical_cast<std::string>(size);
    str_str_map["ndim_" + counter_str] = boost::lexical_cast<std::string>(ndim);
    str_str_map["gdim_" + counter_str] = gdim;
    str_str_map["lb_" + counter_str] = lb;
    str_str_map["ub_" + counter_str] = ub;
    
    counter++;
  }
  
  return str_str_map;
}

int RQTable::mark_all()
{
  for (std::map<key_ver_pair, bool>::iterator it = key_ver__mark.begin(); it != key_ver__mark.end(); it++)
  {
    it->second = true;
  }
  
  return 0;
}

std::map<std::string, std::string> RQTable::to_unmarked_str_str_map()
{
  std::map<std::string, std::string> str_str_map;
  
  int counter = 0;
  for (std::map<key_ver_pair, char>::iterator it = key_ver__dsid_map.begin(); it != key_ver__dsid_map.end(); it++) 
  {
    key_ver_pair kv = it->first;
    if (!key_ver__mark[kv]) {
      std::string key = kv.first;
      unsigned int ver = kv.second;
      char ds_id = it->second;
      
      std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
      uint64_t size = datainfo_map["size"].back();
      int ndim = (int)datainfo_map["ndim"].back();
      std::string gdim = "";
      std::string lb = "";
      std::string ub = "";
      for(int i = 0; i < ndim; i++) {
        gdim += boost::lexical_cast<std::string>(datainfo_map["gdim"][i]);
        lb += boost::lexical_cast<std::string>(datainfo_map["lb"][i]);
        ub += boost::lexical_cast<std::string>(datainfo_map["ub"][i]);
        if (i < ndim-1) {
          gdim += ",";
          lb += ",";
          ub += ",";
        }
      }
      //
      std::string counter_str = boost::lexical_cast<std::string>(counter);
      str_str_map["key_" + counter_str] = key;
      str_str_map["ver_" + counter_str] = boost::lexical_cast<std::string>(ver);
      str_str_map["ds_id_" + counter_str] = ds_id;
      str_str_map["size_" + counter_str] = boost::lexical_cast<std::string>(size);
      str_str_map["ndim_" + counter_str] = boost::lexical_cast<std::string>(ndim);
      str_str_map["gdim_" + counter_str] = gdim;
      str_str_map["lb_" + counter_str] = lb;
      str_str_map["ub_" + counter_str] = ub;
      
      counter++;
    }
  }
  
  return str_str_map;
}

//*************************************  RSTable  ********************************//
RSTable::RSTable() { }
RSTable::~RSTable() { }

int RSTable::push_subscriber(std::string key, unsigned int ver, char ds_id)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (key_ver__ds_id_vector_map.contains(kv) ) {
    key_ver__ds_id_vector_map[kv].push_back(ds_id);
  }
  else {
    std::vector<char> v;
    key_ver__ds_id_vector_map[kv] = v;
    key_ver__ds_id_vector_map[kv].push_back(ds_id);
  }
  
  return 0;
}

int RSTable::pop_subscriber(std::string key, unsigned int ver, char& ds_id)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (!key_ver__ds_id_vector_map.contains(kv) ) {
    return 1;
  }
  ds_id = key_ver__ds_id_vector_map[kv].back();
  key_ver__ds_id_vector_map[kv].pop_back();
  
  if (key_ver__ds_id_vector_map[kv].empty() ) {
    key_ver__ds_id_vector_map.del(kv);
  }
  
  return 0;
}

//*************************************  GFTPBTable  ********************************//
GFTPBTable::GFTPBTable() { }
GFTPBTable::~GFTPBTable() { }

int GFTPBTable::add(char ds_id, std::string laddr, std::string lport, std::string tmpfs_dir)
{
  if (dsid_laddr_map.contains(ds_id) ) {
    LOG(INFO) << "add:: already in, won't over-write; ds_id= " << ds_id;
    return 0;
  }
  dsid_laddr_map[ds_id] = laddr;
  dsid_lport_map[ds_id] = lport;
  dsid_tmpfsdir_map[ds_id] = tmpfs_dir;
  // 
  LOG(INFO) << "add:: done for <ds_id= " << ds_id << ", (laddr= " << laddr << ", lport= " << lport << ", tmpfs_dir= " << tmpfs_dir << ")>";
  return 0;
}

int GFTPBTable::del(char ds_id) {
  if (!dsid_laddr_map.contains(ds_id) ) {
    LOG(ERROR) << "del:: does not exist!; ds_id= " << ds_id;
    return 1;
  }
  dsid_laddr_map.del(ds_id);
  dsid_lport_map.del(ds_id);
  dsid_tmpfsdir_map.del(ds_id);
  // 
  LOG(INFO) << "del:: done; ds_id= " << ds_id;
  return 0;
}

int GFTPBTable::get(char ds_id, std::string &laddr, std::string &lport, std::string &tmpfs_dir)
{
  if (!dsid_laddr_map.contains(ds_id) ) {
    LOG(ERROR) << "get:: does not exist!; ds_id= " << ds_id;
    return 1;
  }
  laddr = dsid_laddr_map[ds_id];
  lport = dsid_lport_map[ds_id];
  tmpfs_dir = dsid_tmpfsdir_map[ds_id];
  return 0;
}

bool GFTPBTable::contains(char ds_id)
{
  return dsid_laddr_map.contains(ds_id);
}

//************************************  RFPManager  ********************************//
RFPManager::RFPManager(std::string wa_trans_protocol, boost::shared_ptr<DSpacesDriver> ds_driver_,
                       std::list<std::string> wa_ib_lport_list, std::string wa_gftp_lintf, 
                       std::string wa_gftp_lport, std::string tmpfs_dir)
: wa_trans_protocol(wa_trans_protocol),
  ds_driver_(ds_driver_),
  ibdd_manager_( boost::make_shared<IBDDManager>(wa_ib_lport_list) )
{
  if ( !(wa_trans_protocol.compare(INFINIBAND) == 0 || wa_trans_protocol.compare(GRIDFTP) == 0) ) {
    LOG(ERROR) << "RFPManager:: unknown wa_trans_protocol= " << wa_trans_protocol;
    exit(1);
  }
#ifdef _GRIDFTP_
  if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    gftpdd_manager_ = boost::make_shared<GFTPDDManager>(wa_gftp_lintf, boost::lexical_cast<int>(wa_gftp_lport), tmpfs_dir);
    // boost::make_shared<boost::thread>(&RFPManager::init_gftp_server, this);
    // boost::thread(&RFPManager::init_gftp_server, this);
  }
#endif
  LOG(INFO) << "RFPManager:: constructed.";
}

RFPManager::~RFPManager()
{
  LOG(INFO) << "RFPManager:: destructed.";
}

int RFPManager::wa_get(std::string laddr, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_) {
  if (wa_trans_protocol.compare(INFINIBAND) == 0) {
    return ib_receive__ds_put(laddr, lport, key, ver, data_type, size, ndim, gdim_, lb_, ub_);
  }
#ifdef _GRIDFTP_
  else if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    // return 0;
    // return gftp_get__ds_put(laddr, lport, tmpfs_dir, key, ver, size, ndim, gdim_, lb_, ub_);
    return gftpfile_read__ds_put(key, ver, size, ndim, gdim_, lb_, ub_);
  }
#endif
  else {
    LOG(ERROR) << "wa_get:: unknown wa_trans_protocol= " << wa_trans_protocol;
    return 1;
  }
}

int RFPManager::wa_put(std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                       std::string laddr, std::string lport, std::string tmpfs_dir) {
  if (wa_trans_protocol.compare(INFINIBAND) == 0) {
    return ds_get__ib_send(key, ver, data_type, size, ndim, gdim_, lb_, ub_, laddr.c_str(), lport.c_str() );
  }
#ifdef _GRIDFTP_
  else if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    return ds_get__gftp_put(key, ver, size, ndim, gdim_, lb_, ub_, laddr, lport, tmpfs_dir);
  }
#endif
  else {
    LOG(ERROR) << "wa_put:: unknown wa_trans_protocol= " << wa_trans_protocol;
    return 1;
  }
}

#ifdef _GRIDFTP_
void RFPManager::init_gftp_server()
{
  if (gftpdd_manager_->init_gftp_server() ) {
    LOG(ERROR) << "RFPManager:: gftpdd_manager_->init_gftp_server failed!";
  }
}

int RFPManager::gftp_get__ds_put(std::string gftps_laddr, std::string gftps_lport, std::string gftps_tmpfs_dir,
                                 std::string key, unsigned int ver,
                                 int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_) 
{
  LOG(INFO) << "gftp_get__ds_put:: started for <key= " << key << ", ver= " << ver << ">, gftps_laddr= " << gftps_laddr << ", gftps_lport= " << gftps_lport << ", gftps_tmpfs_dir= " << gftps_tmpfs_dir;
  size_t datasize_inB;
  void* data_;
  if (gftpdd_manager_->get_over_gftp(gftps_laddr, gftps_lport, gftps_tmpfs_dir,
                                     key, ver, datasize_inB, data_) ) {
    LOG(ERROR) << "gftp_get__ds_put:: gftpdd_manager_->get_over_gftp failed!";
    return 1;
  }
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "gftp_get__ds_put:: ds_driver_->sync_put failed!";
    return 1;
  }
  free(data_);
  //
  LOG(INFO) << "gftp_get__ds_put:: done.";
  return 0;
}

int RFPManager::ds_get__gftp_put(std::string key, unsigned int ver,
                                 int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                                 std::string gftps_laddr, std::string gftps_lport, std::string gftps_tmpfs_dir)
{
  LOG(INFO) << "ds_get__gftp_put:: started for <key= " << key << ", ver= " << ver << ">, gftps_laddr= " << gftps_laddr << ", gftps_lport= " << gftps_lport;
  size_t data_length = get_data_length(ndim, gdim_, lb_, ub_);
  if (!data_length) {
    LOG(ERROR) << "ds_get__gftp_put:: data_length=0!";
    return 1;
  }
  size_t datasize_inB = size*data_length;
  void* data_ = malloc(datasize_inB);
  // debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "ds_get__gftp_put:: ds_driver_->get for <key= " << key << ", ver= " << ver <<"> failed!";
    return 1;
  }
  
  if (gftpdd_manager_->put_over_gftp(gftps_laddr, gftps_lport, gftps_tmpfs_dir, 
                                     key, ver, datasize_inB, data_) ) {
    LOG(ERROR) << "ds_get__gftp_put:: gftpdd_manager_->put_over_gftp for <key= " << key << ", ver= " << ver <<"> failed!";
    return 1;
  }
  free(data_);
  // 
  LOG(INFO) << "ds_get__gftp_put:: done for <key= " << key << ", ver= " << ver << ">.";
  return 0;
}

int RFPManager::gftpfile_read__ds_put(std::string key, unsigned int ver,
                                      int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "gftpfile_read__ds_put:: started for <key= " << key << ", ver= " << ver << ">.";
  size_t datasize_inB;
  void* data_;
  if (gftpdd_manager_->read_del_datafile(key, ver, datasize_inB, data_) ) {
    LOG(ERROR) << "gftpfile_read__ds_put:: gftpdd_manager_->read_del_datafile failed!";
    return 1;
  }
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    free(data_);
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "gftpfile_read__ds_put:: ds_driver_->sync_put failed!";
    return 1;
  }
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  free(data_);
  // 
  LOG(INFO) << "gftpfile_read__ds_put:: done for <key= " << key << ", ver= " << ver << ">.";
  return 0;
}
#endif

std::string RFPManager::get_lport()
{
  if (wa_trans_protocol.compare(INFINIBAND) == 0) {
    return ibdd_manager_->get_next_avail_ib_lport();
  }
#ifdef _GRIDFTP_
  else if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    return boost::lexical_cast<std::string>(gftpdd_manager_->get_gftps_port() );
  }
#endif
  return "";
}

int RFPManager::ib_receive__ds_put(std::string ib_laddr, std::string ib_lport,
                                   std::string key, unsigned int ver, std::string data_type, 
                                   int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "ib_receive__ds_put:: started for <key= " << key << ", ver= " << ver << ">, ib_laddr= " << ib_laddr << ", ib_lport= " << ib_lport;
  key_ver_pair kv = std::make_pair(key, ver);
  key_ver__recvedsize_map[kv] = 0;
  key_ver__data_map[kv] = malloc(size*get_data_length(ndim, gdim_, lb_, ub_) );
  
  ibdd_manager_->init_ib_server(key, ver, data_type, ib_lport.c_str(), 
                                boost::bind(&RFPManager::handle_ib_receive, this, _1, _2, _3, _4) );
  ibdd_manager_->give_ib_lport_back(ib_lport);
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, key_ver__data_map[kv]) ) {
    LOG(ERROR) << "ib_receive__ds_put:: ds_driver_->sync_put failed!";
    return 1;
  }
  
  free(key_ver__data_map[kv]);
  
  std::map<key_ver_pair, void*>::iterator key_ver__data_map_itr = key_ver__data_map.find(kv);
  key_ver__data_map.erase(key_ver__data_map_itr);
  std::map<key_ver_pair, size_t>::iterator key_ver__recvedsize_map_itr = key_ver__recvedsize_map.find(kv);
  key_ver__recvedsize_map.erase(key_ver__recvedsize_map_itr);
  //
  LOG(INFO) << "ib_receive__ds_put:: done.";
  return 0;
}

void RFPManager::handle_ib_receive(std::string key, unsigned int ver, size_t data_size, void* data_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!key_ver__recvedsize_map.count(kv) ) {
    LOG(ERROR) << "handle_ib_receive:: data is received for an unexpected <key= " << key << ", ver= " << ver << ">";
    return;
  }
  
  size_t recved_size = key_ver__recvedsize_map[kv];
  LOG(INFO) << "handle_ib_receive:: for <key= " << key << ", ver= " << ver << ">, recved data_size= " << data_size << ", total_received_size= " << (float)(recved_size+data_size)/1024/1024 << "(MB)";
  
  char* data_t_ = static_cast<char*>(key_ver__data_map[kv]);
  memcpy(data_t_+recved_size, data_, data_size);
  
  key_ver__recvedsize_map[kv] += data_size;
  
  //
  // LOG(INFO) << "handle_ib_receive:: key_ver__data_map[key]=";
  // for(int i=0; i<(data_size/size); i++){
  //     std::cout << static_cast<int*>(key_ver__data_map[key])[i] << ", ";
  // }
  // std::cout << "\n";
  //
  free(data_);
}

int RFPManager::ds_get__ib_send(std::string key, unsigned int ver, std::string data_type, 
                                int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                                const char* ib_laddr, const char* ib_lport)
{
  LOG(INFO) << "ds_get__ib_send:: started for <key= " << key << ", ver= " << ver << ">, ib_laddr= " << ib_laddr << ", ib_lport= " << ib_lport;
  size_t data_length = get_data_length(ndim, gdim_, lb_, ub_);
  if (!data_length) {
    LOG(ERROR) << "ds_get__ib_send:: data_length=0!";
    return 1;
  }
  void* data_ = malloc(size*data_length);
  // debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "ds_get__ib_send:: ds_driver_->get for <key= " << key << ", ver= " << ver <<"> failed!";
    return 1;
  }
  
  ibdd_manager_->init_ib_client(ib_laddr, ib_lport, data_type, data_length, data_);
  free(data_);
  // 
  LOG(INFO) << "ds_get__ib_send:: done.";
  return 0;
}

size_t RFPManager::get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for(int i = 0; i < ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i]) {
      LOG(ERROR) << "get_data_length:: lb= " << lb << " is not feasible!";
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb) {
      LOG(ERROR) << "get_data_length:: ub= " << ub << " is not feasible!";
      return 0;
    }
    dim_length[i] = ub - lb+1;
  }
  
  size_t volume = 1;
  for(int i = 0; i < ndim; i++) {
    volume *= (size_t)dim_length[i];
  }
  
  return volume;
}
//******************************************  RIManager ******************************************//
RIManager::RIManager(char id, int num_cnodes, int app_id,
                     char* dht_lip, int dht_lport, char* ipeer_dht_lip, int ipeer_dht_lport,
                     std::string wa_trans_protocol, std::string wa_laddr, std::string wa_gftp_lintf, std::string wa_gftp_lport, 
                     std::string tmpfs_dir, std::list<std::string> wa_ib_lport_list)
: id(id),
  num_cnodes(num_cnodes),
  app_id(app_id),
  wa_trans_protocol(wa_trans_protocol),
  wa_gftp_lintf(wa_gftp_lintf),
  wa_laddr(wa_laddr),
  wa_gftp_lport(wa_gftp_lport),
  tmpfs_dir(tmpfs_dir),
  dht_node_(new DHTNode(id, boost::bind(&RIManager::handle_wamsg, this, _1),
                        dht_lip, dht_lport, ipeer_dht_lip, ipeer_dht_lport) ),
  ds_driver_( new DSpacesDriver(num_cnodes, app_id) ),
  bc_server_(new BCServer(app_id, num_cnodes, APP_RIMANAGER_MAX_MSG_SIZE, "req_app_", 
                          boost::bind(&RIManager::handle_app_req, this, _1),
                          ds_driver_) ),
  rfp_manager_(new RFPManager(wa_trans_protocol, ds_driver_, wa_ib_lport_list, wa_gftp_lintf, wa_gftp_lport, tmpfs_dir) )
{
  bc_server_->init_listen_all();
  //
  LOG(INFO) << "RIManager:: constructed; \n" << to_str();
}

RIManager::~RIManager()
{
  //dht_node_->close();
  //
  LOG(INFO) << "RIManager:: destructed.";
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t id= " << id << "\n";
  ss << "\t num_cnodes= " << num_cnodes << "\n";
  ss << "\t app_id= " << app_id << "\n";
  ss << "\t wa_trans_protocol= " << wa_trans_protocol << "\n";
  ss << "\t wa_laddr= " << wa_laddr << "\n";
  ss << "\t wa_gftp_lport= " << wa_gftp_lport << "\n";
  ss << "\t dht_node=\n" << dht_node_->to_str() << "\n";
  //
  return ss.str();
}

int RIManager::bcast_rq_table()
{
  // std::map<std::string, std::string> rq_table_map = rq_table.to_str_str_map();
  std::map<std::string, std::string> rq_table_map = rq_table.to_unmarked_str_str_map();
  rq_table.mark_all();
  
  rq_table_map["type"] = REMOTE_RQTABLE;
  rq_table_map["id"] = id;
  
  return broadcast_msg(RIMSG, rq_table_map);
}

/********* handle app_req *********/
void RIManager::handle_app_req(char* app_req)
{
  std::map<std::string, std::string> app_req_map = imsg_coder.decode(app_req);
  //
  int app_id = boost::lexical_cast<int>(app_req_map["app_id"]);
  appid_bcclient_map[app_id] = boost::make_shared<BCClient>(app_id, num_cnodes, APP_RIMANAGER_MAX_MSG_SIZE, "reply_app_", ds_driver_);
  //
  std::string type = app_req_map["type"];
  
  if (type.compare(GET) == 0) {
    handle_get(false, app_id, app_req_map);
  }
  else if (type.compare(BLOCKING_GET) == 0) {
    handle_get(true, app_id, app_req_map);
  }
  else if (type.compare(PUT) == 0) {
    handle_put(app_req_map);
    bc_server_->reinit_listen_client(app_id);
  }
  else {
    LOG(ERROR) << "handle_app_req:: unknown type= " << type;
  }
  // 
  // bc_server_->reinit_listen_client(app_id);
}

void RIManager::handle_get(bool blocking, int app_id, std::map<std::string, std::string> get_map)
{
  LOG(INFO) << "handle_get:: get_map=";
  print_str_map(get_map);
  
  std::string key = get_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(get_map["ver"]);
  
  std::map<std::string, std::string> reply_msg_map;
  if (blocking) {
    reply_msg_map["type"] = BLOCKING_GET_REPLY;
  }
  else {
    reply_msg_map["type"] = GET_REPLY;
  }
  reply_msg_map["key"] = key;
  reply_msg_map["ver"] = get_map["ver"];
  
  std::string dummy_str;
  char ds_id;
  int dummy = -1;
  uint64_t* dummy_;
  if (rq_table.get_key_ver(key, ver, dummy_str, ds_id, dummy, dummy, dummy_, dummy_, dummy_) ) {
    handle_r_get(blocking, app_id, get_map, reply_msg_map);
  }
  else {
    if (ds_id == this->id) {
      LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> exists in local ds_id= " << ds_id;
      reply_msg_map["ds_id"] = ds_id;
      appid_bcclient_map[app_id]->send(reply_msg_map);
      bc_server_->reinit_listen_client(app_id);
    }
    else {
      LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> exists in remote ds_id= " << ds_id;
      handle_r_get(blocking, app_id, get_map, reply_msg_map);
    }
  }
  //
  LOG(INFO) << "handle_get:: done; blocking= " << blocking;
}

void RIManager::handle_r_get(bool blocking, int app_id, std::map<std::string, std::string> r_get_map, 
                             std::map<std::string, std::string> reply_msg_map)
{
  LOG(INFO) << "handle_r_get:: started for <key= " << r_get_map["key"] << ", ver= " << r_get_map["ver"] << ">.";
  bool failed = false;
  bool is_key_ver_local = false;
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if(imsg_coder.decode_msg_map(r_get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_r_get:: decode_msg_map failed!";
    failed = true;
  }
  
  char ds_id;
  int dummy = -1;
  uint64_t* dummy_;
  if (rq_table.get_key_ver(key, ver, data_type, ds_id, dummy, dummy, dummy_, dummy_, dummy_) ) {
    if (remote_query(blocking, key, ver) ) {
      LOG(ERROR) << "handle_r_get:: remote_query failed!";
      failed = true;
    }
    if (rq_table.get_key_ver(key, ver, data_type, ds_id, dummy, dummy, dummy_, dummy_, dummy_) ) {
      ds_id = '?';
    }
  }
  
  if (ds_id == '?') {
    if (blocking) {
      key_ver_pair kv = std::make_pair(key, ver);
      b_r_get_syncer.add_sync_point(kv, 1);
      b_r_get_syncer.wait(kv);
      b_r_get_syncer.del_sync_point(kv);
      
      ds_id = this->id;
      is_key_ver_local = true;
    }
    else {
      LOG(INFO) << "handle_r_get:: <key= " << key << ", ver= " << ver << "> does not exist.";
    }
  }
  else {
    LOG(INFO) << "handle_r_get:: key= " << key << " exists in ds_id= " << ds_id << ".";
    if (remote_fetch(ds_id, r_get_map) ) {
      LOG(INFO) << "handle_r_get:: remote_fetch failed!";
      ds_id = '?';
      failed = true;
    }
    else {
      is_key_ver_local = true;
    }
  }
  
  if (!failed && is_key_ver_local) {
    rq_table.add_key_ver(key, ver, data_type, this->id, size, ndim, gdim_, lb_, ub_);
  }
  
  reply_msg_map["ds_id"] = ds_id;
  appid_bcclient_map[app_id]->send(reply_msg_map);
  bc_server_->reinit_listen_client(app_id);
  //
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_r_get:: done for <key= " << r_get_map["key"] << ", ver= " << r_get_map["ver"] << ">.";
}

void RIManager::handle_put(std::map<std::string, std::string> put_map)
{
  LOG(INFO) << "handle_put:: started for <key= " << put_map["key"] << ", ver= " << put_map["ver"] << ">.";
  bool failed = false;
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if(imsg_coder.decode_msg_map(put_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_l_put:: decode_msg_map failed!";
  }
  else {
    boost::thread t(&RIManager::handle_possible_remote_places, this, key, ver);
    
    //debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL);
    rq_table.put_key_ver(key, ver, data_type, this->id, size, ndim, gdim_, lb_, ub_);
    if (bcast_rq_table() ) {
      LOG(ERROR) << "handle_l_put:: bcast_rq_table failed!";
    }
  }
  //
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_put:: done for <key= " << put_map["key"] << ", ver= " << put_map["ver"] << ">.";
  // LOG(INFO) << "handle_put:: rq_table=\n" << rq_table.to_str();
  // bc_server_->reinit_listen_client(app_id);
}

// void RIManager::handle_early_subscribe(int app_id, std::map<std::string, std::string> early_subs_map)
// {
  
// }

/*************************************************************************/
void RIManager::handle_possible_remote_places(std::string key, unsigned int ver)
{
  LOG(INFO) << "handle_possible_remote_places:: started for <key= " << key << ", ver= " << ver << ">.";
  
  int num_rps = 0;
  char ds_id;
  while (!rs_table.pop_subscriber(key, ver, ds_id) ) {
    boost::thread t(&RIManager::remote_place, this, key, ver, ds_id);
    num_rps++;
  }
  
  if (num_rps) {
    key_ver_pair kv = std::make_pair(key, ver);
    handle_rp_syncer.add_sync_point(kv, num_rps);
    handle_rp_syncer.wait(kv);
    handle_rp_syncer.del_sync_point(kv);
  }
  //
  LOG(INFO) << "handle_possible_remote_places:: done for <key= " << key << ", ver= " << ver << ">.";
}

//PI: a <key, ver> pair cannot be produced in multiple dataspaces
int RIManager::remote_query(bool subscribe, std::string key, unsigned int ver)
{
  LOG(INFO) << "remote_query:: started;";
  
  std::map<std::string, std::string> r_q_map;
  if (subscribe) {
    r_q_map["type"] = REMOTE_BLOCKING_QUERY;
  }
  else {
    r_q_map["type"] = REMOTE_QUERY;
  }
  r_q_map["key"] = key;
  r_q_map["ver"] = boost::lexical_cast<std::string>(ver);
  
  if (broadcast_msg(RIMSG, r_q_map) ){
    LOG(ERROR) << "remote_query:: broadcast_msg failed!";
    return 1;
  }
  
  key_ver_pair kv = std::make_pair(key, ver);
  rq_syncer.add_sync_point(kv, dht_node_->get_num_peers() );
  rq_syncer.wait(kv);
  rq_syncer.del_sync_point(kv);
  
  LOG(INFO) << "remote_query:: done.";
  return 0;
}

int RIManager::broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->broadcast_msg(msg_type, msg_map);
}

int RIManager::send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->send_msg(ds_id, msg_type, msg_map);
}

int RIManager::remote_subscribe(std::string key, unsigned int ver)
{
  LOG(INFO) << "remote_subscribe:: started for <key= " << key << ", ver= " << ver << ">";
  
  std::map<std::string, std::string> r_subs_map;
  r_subs_map["id"] = id;
  r_subs_map["key"] = key;
  r_subs_map["ver"] = boost::lexical_cast<std::string>(ver);
  
  if (broadcast_msg(RIMSG, r_subs_map) ){
    LOG(ERROR) << "remote_subscribe:: broadcast_msg failed!";
  }
  //
  LOG(INFO) << "remote_subscribe:: done.";
  return 0;
}

int RIManager::remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map)
{
  LOG(INFO) << "remote_fetch:: started;";
  
  std::string lport = rfp_manager_->get_lport(); //Returns either next avail ib_lport or constant gftps_lport
  r_fetch_map["laddr"] = this->wa_laddr;
  r_fetch_map["lport"] = lport;
  r_fetch_map["tmpfs_dir"] = this->tmpfs_dir;
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0) {
    boost::thread t(&RIManager::handle_wa_get, this, "remote_fetch", r_fetch_map );
  }
  
  r_fetch_map["type"] = REMOTE_FETCH;
  if (send_msg(ds_id, RIMSG, r_fetch_map) ) {
    LOG(ERROR) << "remote_fetch:: send_msg to to_id= " << ds_id << " failed!";
    return 1;
  }
  // TODO: sync will not have any affect for gridftp case since wa_get will return immediately
  std::string key = r_fetch_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(r_fetch_map["ver"]);
  key_ver_pair kv = std::make_pair(key, ver);
  rf_wa_get_syncer.add_sync_point(kv, 1);
  rf_wa_get_syncer.wait(kv);
  rf_wa_get_syncer.del_sync_point(kv);
#ifdef _GRIDFTP_
  if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
    int size, ndim;
    uint64_t *gdim_, *lb_, *ub_;
    std::string data_type;
    if (imsg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      LOG(ERROR) << "remote_fetch:: imsg_coder.decode_msg_map failed!";
      return 1;
    }
    if (rfp_manager_->gftpfile_read__ds_put(key, ver, size, ndim, gdim_, lb_, ub_) ) {
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      LOG(ERROR) << "remote_fetch:: rfp_manager_->gftpfile_read__ds_put failed!";
      return 1;
    }
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
  }
#endif
  // 
  LOG(INFO) << "remote_fetch:: done.";
  return 0;
}

int RIManager::remote_place(std::string key, unsigned int ver, char to_id)
{
  LOG(INFO) << "remote_place:: started for <key= " << key << ", ver= " << ver << "> --> to_id= " << to_id;
  
  char ds_id;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (rq_table.get_key_ver(key, ver, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
    LOG(INFO) << "remote_place:: rq_table.get_key_ver failed!";
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
  }
  
  // debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  std::map<std::string, std::string> r_place_map;
  r_place_map["type"] = REMOTE_PLACE;
  r_place_map["id"] = id;
  if (imsg_coder.encode_msg_map(r_place_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_place:: encode_msg_map failed!";
    return 1;
  }
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0) {
    if (send_msg(to_id, RIMSG, r_place_map) ) {
      LOG(ERROR) << "remote_place:: send_msg to to_id= " << to_id << " failed!";
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      return 1;
    }
    
    key_ver_pair kv = std::make_pair(key, ver);
    rp_syncer.add_sync_point(kv, 1 );
    rp_syncer.wait(kv);
    rp_syncer.del_sync_point(kv);
    
    laddr_lport__tmpfsdir_pair ll_t = key_ver___laddr_lport__tmpfsdir_map[kv];
    if (rfp_manager_->wa_put(key, ver, data_type, size, ndim, gdim_, lb_, ub_, ll_t.first.first, ll_t.first.second, ll_t.second) ) {
      LOG(ERROR) << "remote_place:: rfp_manager_->wa_put failed!";
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      return 1;
    }
    handle_rp_syncer.notify(kv);
  }
  else if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
    if (!gftpb_table.contains(to_id) ) {
      gftpb_ping(to_id);
    }
    std::string laddr, lport, tmpfs_dir;
    if (gftpb_table.get(to_id, laddr, lport, tmpfs_dir) ) {
      LOG(ERROR) << "remote_place:: gftpb_table.get failed!";
      return 1;
    }
    
    if (rfp_manager_->wa_put(key, ver, "", size, ndim, gdim_, lb_, ub_, laddr, lport, tmpfs_dir) ) {
      LOG(ERROR) << "remote_place:: rfp_manager_->wa_put failed!";
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      return 1;
    }
    if (send_msg(to_id, RIMSG, r_place_map) ) {
      LOG(ERROR) << "remote_place:: send_msg to to_id= " << to_id << " failed!";
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      return 1;
    }
  }
  //
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "remote_place:: done.";
  return 0;
}

void RIManager::handle_wa_get(std::string called_from, std::map<std::string, std::string> str_str_map)
{
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (imsg_coder.decode_msg_map(str_str_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "handle_wa_get:: decode_msg_map failed!";
    return;
  }
  // 
  LOG(INFO) << "handle_wa_get:: debug_print=";
  debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  // 
  if (rfp_manager_->wa_get(str_str_map["laddr"], str_str_map["lport"], str_str_map["tmpfs_dir"],
                           key, ver, data_type, 
                           size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_wa_get:: rfp_manager_->wa_get failed!";
  }
  
  if (called_from.compare("remote_fetch") == 0) {
    rf_wa_get_syncer.notify(std::make_pair(key, ver) );
  }
  else if (called_from.compare("handle_r_place") == 0) {
    rp_wa_get_syncer.notify(std::make_pair(key, ver) );
  }
  
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
}

int RIManager::gftpb_ping(char to_id)
{
  LOG(INFO) << "gftpb_ping:: started for to_id= " << to_id;
  
  std::map<std::string, std::string> gftpb_ping_map;
  gftpb_ping_map["type"] = GFTPB_PING;
  gftpb_ping_map["id"] = this->id;
  
  if (send_msg(to_id, RIMSG, gftpb_ping_map) ) {
    LOG(ERROR) << "gftpb_ping:: send_msg to to_id= " << to_id << " failed!";
    return 1;
  }
  gftpb_ping_syncer.add_sync_point(to_id, 1 );
  gftpb_ping_syncer.wait(to_id);
  gftpb_ping_syncer.del_sync_point(to_id);
  //
  LOG(INFO) << "gftpb_ping:: done.";
  return 0;
}
/********* handle wamsg *********/
void RIManager::handle_wamsg(std::map<std::string, std::string> wamsg_map)
{
  std::string type = wamsg_map["type"];
  
  if (type.compare(REMOTE_QUERY) == 0) {
    handle_r_query(false, wamsg_map);
  }
  else if (type.compare(REMOTE_BLOCKING_QUERY) == 0) {
    handle_r_query(true, wamsg_map);
  }
  else if (type.compare(REMOTE_QUERY_REPLY) == 0) {
    handle_rq_reply(wamsg_map);
  }
  else if (type.compare(REMOTE_FETCH) == 0) {
    handle_r_fetch(wamsg_map);
  }
  else if (type.compare(REMOTE_RQTABLE) == 0) {
    handle_r_rqtable(wamsg_map);
  }
  else if (type.compare(REMOTE_PLACE) == 0) {
    handle_r_place(wamsg_map);
  }
  else if (type.compare(REMOTE_PLACE_REPLY) == 0) {
    handle_rp_reply(wamsg_map);
  }
  else if (type.compare(GFTPPUT_DONE) == 0) {
    handle_gftpput_done(wamsg_map);
  }
  else if (type.compare(GFTPB_PING) == 0) {
    handle_gftpb_ping(wamsg_map);
  }
  else if (type.compare(GFTPB_PONG) == 0) {
    handle_gftpb_pong(wamsg_map);
  }
  else{
    LOG(ERROR) << "handle_wamsg:: unknown type= " << type;
  }
  //
  LOG(INFO) << "handle_wamsg:: done.";
}

void RIManager::handle_r_query(bool subscribe, std::map<std::string, std::string> r_query_map)
{
  LOG(INFO) << "handle_r_query:: r_query_map=";
  print_str_map(r_query_map);
  
  std::string key = r_query_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(r_query_map["ver"]);
  
  std::map<std::string, std::string> rq_reply_map;
  rq_reply_map["type"] = REMOTE_QUERY_REPLY;
  rq_reply_map["key"] = key;
  rq_reply_map["ver"] = r_query_map["ver"];
  
  char to_id = r_query_map["id"].c_str()[0];
  std::string data_type;
  char ds_id;
  int dummy = -1;
  uint64_t* dummy_;
  if (rq_table.get_key_ver(key, ver, data_type, ds_id, dummy, dummy, dummy_, dummy_, dummy_) ) {
    //LOG(INFO) << "handle_r_query:: does not exist; key= " << key;
    rq_reply_map["ds_id"] = '?';
    
    if (subscribe) {
      rs_table.push_subscriber(key, ver, to_id);
      LOG(INFO) << "handle_r_query:: id= " << to_id << " got subscribed for <key= " << key << ", ver= " << ver << ">";
    }
  }
  else{
    rq_reply_map["ds_id"] = ds_id;
  }
  
  if(send_msg(to_id, RIMSG, rq_reply_map) ) {
    LOG(ERROR) << "handle_r_query:: send_msg to to_id= " << to_id << " failed!";
  }
  //
  LOG(INFO) << "handle_r_query:: done.";
}

void RIManager::handle_rq_reply(std::map<std::string, std::string> rq_reply_map)
{
  LOG(INFO) << "handle_rq_reply:: rq_reply_map=";
  print_str_map(rq_reply_map);
  
  std::string key = rq_reply_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(rq_reply_map["ver"]);
  std::string data_type = rq_reply_map["data_type"];
  char ds_id = rq_reply_map["ds_id"].c_str()[0];
  if (ds_id != '?'){
    rq_table.put_key_ver(key, ver, data_type, ds_id, 0, 0, NULL, NULL, NULL);
  }
  
  rq_syncer.notify( std::make_pair(key, ver) );
  //
  LOG(INFO) << "handle_rq_reply:: done.";
}

void RIManager::handle_r_fetch(std::map<std::string, std::string> r_fetch_map)
{
  LOG(INFO) << "handle_r_fetch:: r_fetch_map=";
  print_str_map(r_fetch_map);
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (imsg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "handle_r_fetch:: decode_msg_map failed!";
    return;
  }
  
  if (rfp_manager_->wa_put(key, ver, data_type, size, ndim, gdim_, lb_, ub_,
                            r_fetch_map["laddr"], r_fetch_map["lport"], r_fetch_map["tmpfs_dir"] ) ) {
    LOG(ERROR) << "handle_r_fetch:: rfp_manager_->wa_put failed!";
  }
  // 
#ifdef _GRIDFTP_
  if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    char to_id = r_fetch_map["id"].c_str()[0];
    std::map<std::string, std::string> gftp_put_done_map;
    gftp_put_done_map["type"] = GFTPPUT_DONE;
    gftp_put_done_map["id"] = to_id;
    gftp_put_done_map["key"] = key;
    gftp_put_done_map["ver"] = boost::lexical_cast<std::string>(ver);
    if (send_msg(to_id, RIMSG, gftp_put_done_map) ) {
      LOG(ERROR) << "handle_r_fetch:: send_msg to to_id= " << to_id << " failed!";
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      return;
    }
  }
#endif
  //
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_r_fetch:: done.";
}

void RIManager::handle_r_rqtable(std::map<std::string, std::string> r_rqtable_map)
{
  LOG(INFO) << "handle_r_rqtable:: r_rqtable_map=";
  print_str_map(r_rqtable_map);
  
  int count = 0;
  while(1) {
    std::string tail_str = boost::lexical_cast<std::string>(count);
    std::string key_str = "key_" + tail_str;
    if (!r_rqtable_map.count(key_str) ) {
      break;
    }
    
    int ndim = boost::lexical_cast<int>(r_rqtable_map["ndim_"+tail_str]);
    uint64_t* gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    uint64_t* lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    uint64_t* ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    
    
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > gdim_tokens(r_rqtable_map["gdim_"+tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator gdim_tokens_it = gdim_tokens.begin();
    boost::tokenizer<boost::char_separator<char> > lb_tokens(r_rqtable_map["lb_"+tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator lb_tokens_it = lb_tokens.begin();
    boost::tokenizer<boost::char_separator<char> > ub_tokens(r_rqtable_map["ub_"+tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator ub_tokens_it = ub_tokens.begin();
    for (int i = 0; i < ndim; i++) {
      gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_tokens_it);
      gdim_tokens_it++;
      lb_[i] = boost::lexical_cast<uint64_t>(*lb_tokens_it);
      lb_tokens_it++;
      ub_[i] = boost::lexical_cast<uint64_t>(*ub_tokens_it);
      ub_tokens_it++;
    }
    
    rq_table.put_key_ver(r_rqtable_map[key_str], boost::lexical_cast<unsigned int>(r_rqtable_map["ver_"+tail_str]),
                         r_rqtable_map["data_type_"+tail_str], r_rqtable_map["ds_id_"+tail_str].c_str()[0],
                         boost::lexical_cast<int>(r_rqtable_map["size_"+tail_str]),
                         ndim, gdim_, lb_, ub_ );
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
    
    count++;
  }
  //
  LOG(INFO) << "handle_rq_reply:: done.";
}

void RIManager::handle_r_place(std::map<std::string, std::string> r_place_map)
{
  LOG(INFO) << "handle_r_place:: r_place_map=";
  print_str_map(r_place_map);
  
  std::string lport = rfp_manager_->get_lport();
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (imsg_coder.decode_msg_map(r_place_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    free_all<uint64_t*>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "handle_r_place:: decode_msg_map failed!";
    return;
  }
  
  key_ver_pair kv = std::make_pair(key, ver);
  rp_wa_get_syncer.add_sync_point(kv, 1);
  
  r_place_map["laddr"] = wa_laddr;
  r_place_map["lport"] = lport;
  boost::thread t(&RIManager::handle_wa_get, this, "handle_r_place", r_place_map);
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0) {
    std::map<std::string, std::string> rp_reply_map;
    rp_reply_map["type"] = REMOTE_PLACE_REPLY;
    rp_reply_map["key"] = key;
    rp_reply_map["ver"] = r_place_map["ver"];
    rp_reply_map["laddr"] = wa_laddr;
    rp_reply_map["lport"] = lport;
    char to_id = r_place_map["id"].c_str()[0];
    if (send_msg(to_id, RIMSG, rp_reply_map) ) {
      free_all<uint64_t*>(3, gdim_, lb_, ub_);
      LOG(ERROR) << "handle_r_place:: send_msg to to_id= " << to_id << " failed!";
      return;
    }
  }
  // 
  rp_wa_get_syncer.wait(kv);
  rp_wa_get_syncer.del_sync_point(kv);
  
  rq_table.add_key_ver(key, ver, data_type, this->id, size, ndim, gdim_, lb_, ub_);
  
  // TODO: Following may throw exception in case of non-blocking get
  b_r_get_syncer.notify(kv);
  //
  free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_r_place:: done.";
  
  
  // if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
  //   int size, ndim;
  //   uint64_t *gdim_, *lb_, *ub_;
  //   std::string data_type;
  //   if (imsg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
  //     free_all<uint64_t*>(3, gdim_, lb_, ub_);
  //     LOG(ERROR) << "remote_fetch:: imsg_coder.decode_msg_map failed!";
  //     return 1;
  //   }
  //   if (rfp_manager_->gftpfile_read__ds_put(key, ver, size, ndim, gdim_, lb_, ub_) ) {
  //     free_all<uint64_t*>(3, gdim_, lb_, ub_);
  //     LOG(ERROR) << "remote_fetch:: rfp_manager_->gftpfile_read__ds_put failed!";
  //     return 1;
  //   }
  //   free_all<uint64_t*>(3, gdim_, lb_, ub_);
  // }
}

void RIManager::handle_rp_reply(std::map<std::string, std::string> rp_reply_map)
{
  LOG(INFO) << "handle_rp_reply:: rp_reply_map=";
  print_str_map(rp_reply_map);
  
  key_ver_pair kv = std::make_pair(rp_reply_map["key"], boost::lexical_cast<unsigned int>(rp_reply_map["ver"]) );
  key_ver___laddr_lport__tmpfsdir_map[kv] = std::make_pair(std::make_pair(rp_reply_map["laddr"], rp_reply_map["lport"]), rp_reply_map["tmpfs_dir"]);
  
  rp_syncer.notify(kv);
  //
  LOG(INFO) << "handle_rp_reply:: done.";
}

void RIManager::handle_r_subscribe(std::map<std::string, std::string> r_subs_map)
{
  LOG(INFO) << "handle_r_subscribe:: r_subs_map=";
  print_str_map(r_subs_map);
  
  rs_table.push_subscriber(r_subs_map["key"], boost::lexical_cast<unsigned int>(r_subs_map["ver"]), 
                           boost::lexical_cast<char>(r_subs_map["id"]) );
  //
  LOG(INFO) << "handle_r_subscribe:: done.";
}

#ifdef _GRIDFTP_
void RIManager::handle_gftpput_done(std::map<std::string, std::string> gftpput_done_map)
{
  LOG(INFO) << "handle_gftpput_done:: gftpput_done_map=";
  print_str_map(gftpput_done_map);
  
  rf_wa_get_syncer.notify(std::make_pair(gftpput_done_map["key"], boost::lexical_cast<unsigned int>(gftpput_done_map["ver"]) ) );
  //
  LOG(INFO) << "handle_gftpput_done:: done.";
}

void RIManager::handle_gftpb_ping(std::map<std::string, std::string> gftpb_ping_map)
{
  LOG(INFO) << "handle_gftpb_ping:: gftpb_ping_map=";
  print_str_map(gftpb_ping_map);
  
  std::map<std::string, std::string> gftpb_pong_map;
  gftpb_pong_map["type"] = GFTPB_PONG;
  gftpb_pong_map["id"] = this->id;
  gftpb_pong_map["laddr"] = this->wa_laddr;
  gftpb_pong_map["lport"] = this->wa_gftp_lport;
  gftpb_pong_map["tmpfs_dir"] = this->tmpfs_dir;
  
  char to_id = gftpb_ping_map["id"].c_str()[0];
  if (send_msg(to_id, RIMSG, gftpb_pong_map) ) {
    LOG(ERROR) << "handle_gftpb_ping:: send_msg to to_id= " << to_id << " failed!";
    return;
  }
  //
  LOG(INFO) << "handle_gftpb_ping:: done.";
}

void RIManager::handle_gftpb_pong(std::map<std::string, std::string> gftpb_pong_map)
{
  LOG(INFO) << "handle_gftpb_pong:: gftpb_pong_map=";
  print_str_map(gftpb_pong_map);
  
  char ds_id = gftpb_pong_map["id"].c_str()[0];
  gftpb_table.add(ds_id, gftpb_pong_map["laddr"], gftpb_pong_map["lport"], gftpb_pong_map["tmpfs_dir"] );
  gftpb_ping_syncer.notify(ds_id);
  // 
  LOG(INFO) << "handle_gftpb_pong:: done.";
}
#endif
