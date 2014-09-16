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
  if (data == NULL){
    return;
  }
  
  std::cout << "data=";
  for (int i=0; i<data_length; i++){
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}

void print_str_map(std::map<std::string, std::string> str_map)
{
  for (std::map<std::string, std::string>::const_iterator it=str_map.begin(); 
       it!=str_map.end(); ++it){
    std::cout << "\t" << it->first << ":" << it->second << "\n";
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

int IMsgCoder::decode_i_msg(std::map<std::string, std::string> msg_map,
                            std::string& key, unsigned int& ver, int& size,
                            int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  try
  {
    key = msg_map["key"];
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
    LOG(ERROR) << "decode_i_msg:: Exception=" << ex.what();
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
  for(int i=1; i<=num_clients; i++){
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
  uint64_t gdim = max_msg_size;
  uint64_t lb = 0;
  uint64_t ub = max_msg_size-1;
  
  char *data_ = (char*)malloc(max_msg_size*sizeof(char));
  strcpy(data_, msg_str.c_str() );
  for (int i=msg_size; i<max_msg_size-msg_size; i++){
    data_[i] = '\0';
  }
  int result = ds_driver_->sync_put(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  //int result = ds_driver_->sync_put_without_lock(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  //LOG(INFO) << "send:: sync_put_without_lock returned " << result;
  free(data_);
  
  //ds_driver_->lock_on_write(comm_var_name.c_str() );
  
  return 0;
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

bool RQTable::get_key_ver(std::string key, unsigned int ver, 
                          char& ds_id, int* size, int* ndim, 
                          uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (!key_ver__dsid_map.count(kv) ){
    return false;
  }
  ds_id = key_ver__dsid_map[kv];
  
  if (size != NULL)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
    *size = (int)datainfo_map["size"].back();
    *ndim = (int)datainfo_map["ndim"].back();
    for(int i = 0; i < *ndim; i++){
      gdim_[i] = datainfo_map["gdim"][i];
      lb_[i] = datainfo_map["lb"][i];
      ub_[i] = datainfo_map["ub"][i];
    }
  }
  
  return true;
}

int RQTable::put_key_ver(std::string key, unsigned int ver, 
                         char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  if (key_ver__dsid_map.count(kv) ){
    return update_key_ver(key, ver, ds_id, size, ndim, gdim_, lb_, ub_);
  }
  return add_key_ver(key, ver, ds_id, size, ndim, gdim_, lb_, ub_);
}

int RQTable::add_key_ver(std::string key, unsigned int ver, 
                         char ds_id, int size, int ndim, 
                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  key_ver_pair kv = std::make_pair(key, ver);
  
  key_ver__dsid_map[kv] = ds_id;
  
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
                            char ds_id, int size, int ndim, 
                            uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  del_key_ver(key, ver);
  add_key_ver(key, ver, ds_id, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "update_key_ver:: updated <key= " << key << ", ver= " << ver << "> : ds_id= " << ds_id;
  return 0;
}

int RQTable::del_key_ver(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  if (!key_ver__dsid_map.count(kv) ){
    LOG(ERROR) << "del_key:: non-existing kv= <" << key << ", " << ver << ">";
    return 1;
  }
  key_ver__dsid_map_it = key_ver__dsid_map.find(kv);
  key_ver__dsid_map.erase(key_ver__dsid_map_it);
  
  key_ver__datainfo_map_it = key_ver__datainfo_map.find(kv);
  key_ver__datainfo_map.erase(key_ver__datainfo_map_it);
  //
  LOG(INFO) << "del_key:: deleted kv= <" << key << ", " << ver << ">";
  return 0;
}

bool RQTable::is_feasible_to_get(std::string key, unsigned int ver, 
                                 int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  if (!key_ver__datainfo_map.count(kv) ){
    return false;
  }
  
  std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
  int _ndim = (int)datainfo_map["ndim"].back();
  int _size = (int)datainfo_map["size"].back();
  if (ndim != _ndim || size != _size) {
    return false;
  }
  // uint64_t _gdim_[ndim];
  // uint64_t _lb_[ndim];
  // uint64_t _ub_[ndim];
  // for(int i = 0; i < ndim; i++){
  //   _gdim_[i] = datainfo_map["gdim"][i];
  //   _lb_[i] = datainfo_map["lb"][i];
  //   _ub_[i] = datainfo_map["ub"][i];
  // }
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
  
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  for (key_ver__dsid_map_it=key_ver__dsid_map.begin(); key_ver__dsid_map_it!=key_ver__dsid_map.end(); ++key_ver__dsid_map_it){
    key_ver_pair kv = key_ver__dsid_map_it->first;
    
    ss << "<key= " << kv.first << ", ver= " << kv.second << ">\n";
    ss << "\tds_id=" << key_ver__dsid_map_it->second << "\n";

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
  
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  int counter = 0;
  for (key_ver__dsid_map_it=key_ver__dsid_map.begin(); key_ver__dsid_map_it!=key_ver__dsid_map.end(); ++key_ver__dsid_map_it){
    key_ver_pair kv = key_ver__dsid_map_it->first;
    std::string key = kv.first;
    unsigned int ver = kv.second;
    char ds_id = key_ver__dsid_map_it->second;
    
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_ver__datainfo_map[kv];
    uint64_t size = datainfo_map["size"].back();
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

//************************************  RFManager  ********************************//
RFManager::RFManager(std::list<std::string> wa_ib_lport_list, boost::shared_ptr<DSpacesDriver> ds_driver_)
: dd_manager_(new DDManager(wa_ib_lport_list) ),
  ds_driver_(ds_driver_)
{
  LOG(INFO) << "RFManager:: constructed.";
}

RFManager::~RFManager()
{
  LOG(INFO) << "RFManager:: destructed.";
}


std::string RFManager::get_ib_lport()
{
  return dd_manager_->get_next_avail_ib_lport();
}

bool RFManager::receive_put(std::string ib_laddr, std::string ib_lport,
                            std::string data_type, std::string key, unsigned int ver, int size,
                            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "receive_put:: started for key= " << key << ", ib_laddr= " << ib_laddr << ", ib_lport= " << ib_lport;
  key_recvedsize_map[key] = 0;
  key_data_map[key] = malloc(size*get_data_length(ndim, gdim_, lb_, ub_) );
  
  dd_manager_->init_ib_server(key, data_type, ib_lport.c_str(), 
                              boost::bind(&RFManager::handle_ib_receive, this, _1, _2, _3) );
  dd_manager_->give_ib_lport_back(ib_lport);
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, key_data_map[key]) ){
    LOG(ERROR) << "receive_put:: ds_driver_->sync_put failed!";
    return false;
  }
  
  free(key_data_map[key]);
  
  std::map<std::string, void*>::iterator key_data_map_itr = key_data_map.find(key);
  key_data_map.erase(key_data_map_itr);
  std::map<std::string, size_t>::iterator key_recvedsize_map_itr = key_recvedsize_map.find(key);
  key_recvedsize_map.erase(key_recvedsize_map_itr);
  //
  LOG(INFO) << "receive_put:: done.";
  return true;
}

void RFManager::handle_ib_receive(std::string key, size_t data_size, void* data_)
{
  if (!key_recvedsize_map.count(key) ){
    LOG(ERROR) << "handle_ib_receive:: data is received for an unexpected key= " << key;
    return;
  }
  
  size_t recved_size = key_recvedsize_map[key];
  LOG(INFO) << "handle_ib_receive:: for key= " << key << ", recved data_size= " << data_size << ", total_received_size= " << (float)(recved_size+data_size)/1024/1024 << "(MB)";
  
  char* data_t_ = static_cast<char*>(key_data_map[key]);
  memcpy(data_t_+recved_size, data_, data_size);
  
  key_recvedsize_map[key] += data_size;
  
  //
  // LOG(INFO) << "handle_ib_receive:: key_data_map[key]=";
  // for(int i=0; i<(data_size/size); i++){
  //     std::cout << static_cast<int*>(key_data_map[key])[i] << ", ";
  // }
  // std::cout << "\n";
  //
  free(data_);
}

bool RFManager::get_send(std::string data_type, std::string key, unsigned int ver, int size,
                         int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                         const char* ib_laddr, const char* ib_lport)
{
  size_t data_length = get_data_length(ndim, gdim_, lb_, ub_);
  if (!data_length){
    LOG(ERROR) << "get_send:: data_length=0!";
    return false;
  }
  void* data_ = malloc(size*data_length);
  
  debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ){
    LOG(ERROR) << "get_send:: ds_driver_->get for key= " << key << " failed!";
    return false;
  }
  
  dd_manager_->init_ib_client(ib_laddr, ib_lport, data_type, data_length, data_);
  free(data_);
  
  return true;
}

size_t RFManager::get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for(int i=0; i<ndim; i++){
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i]){
      LOG(ERROR) << "get_data_length:: lb= " << lb << " is not feasible!";
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb){
      LOG(ERROR) << "get_data_length:: ub= " << ub << " is not feasible!";
      return 0;
    }
    dim_length[i] = ub - lb+1;
  }
  
  size_t volume = 1;
  for(int i=0; i<ndim; i++){
    volume *= (size_t)dim_length[i];
  }
  
  return volume;
}
//******************************************  RIManager ******************************************//
RIManager::RIManager(char id, int num_cnodes, int app_id,
                     char* lip, int lport, char* ipeer_lip, int ipeer_lport,
                     char* ib_laddr, std::list<std::string> wa_ib_lport_list)
: id(id),
  num_cnodes(num_cnodes),
  app_id(app_id),
  ib_laddr(ib_laddr),
  ds_driver_ ( new DSpacesDriver(num_cnodes, app_id) ),
  li_bc_server_(new BCServer(app_id, num_cnodes, LI_MAX_MSG_SIZE, "li_req_", 
                             boost::bind(&RIManager::handle_li_req, this, _1),
                             ds_driver_) ),
  ri_bc_server_(new BCServer(app_id, num_cnodes, RI_MAX_MSG_SIZE, "ri_req_", 
                             boost::bind(&RIManager::handle_ri_req, this, _1),
                             ds_driver_) ),
  dht_node_(new DHTNode(id, boost::bind(&RIManager::handle_wamsg, this, _1),
                        lip, lport, ipeer_lip, ipeer_lport) ),
  rf_manager_(new RFManager(wa_ib_lport_list, ds_driver_) )
{
  //usleep(WAIT_TIME_FOR_BCCLIENT_DSLOCK);
  
  li_bc_server_->init_listen_all();
  ri_bc_server_->init_listen_all();
  //to avoid problem with bc_server and bc_client sync_with_time
  // boost::shared_ptr<DHTNode> t_sp_(
  //   new DHTNode(id, boost::bind(&RIManager::handle_wamsg, this, _1),
  //               lip, lport, ipeer_lip, ipeer_lport) 
  // );
  // this->dht_node_ = t_sp_;
  //
  LOG(INFO) << "RIManager:: constructed.\n" << to_str();
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
  ss << "\t num_cnodes=" << num_cnodes << "\n";
  ss << "\t app_id=" << app_id << "\n";
  ss << "\t dht_node=\n" << dht_node_->to_str() << "\n";
  //
  return ss.str();
}

int RIManager::bcast_rq_table()
{
  std::map<std::string, std::string> rq_table_map = rq_table.to_str_str_map();
  rq_table_map["type"] = REMOTE_RQTABLE;
  rq_table_map["id"] = id;
  
  return broadcast_msg(RIMSG, rq_table_map);
}
/********* handle li_req *********/
void RIManager::handle_li_req(char* li_req)
{
  std::map<std::string, std::string> li_req_map = imsg_coder.decode(li_req);
  //
  int app_id = boost::lexical_cast<int>(li_req_map["app_id"]);
  li_bc_server_->reinit_listen_client(app_id);
  
  std::string type = li_req_map["type"];
  
  if (type.compare(LOCAL_GET) == 0){
    //
  }
  else if (type.compare(LOCAL_PUT) == 0){
    handle_l_put(li_req_map);
  }
  else{
    LOG(ERROR) << "handle_li_req:: unknown type= " << type;
  }
}

void RIManager::handle_l_put(std::map<std::string, std::string> l_put_map)
{
  LOG(INFO) << "handle_l_put:: l_put_map=";
  print_str_map(l_put_map);
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if(imsg_coder.decode_i_msg(l_put_map, key, ver, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_l_put:: decode_i_msg failed!";
    return;
  }
  //debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL);
  rq_table.put_key_ver(key, ver, this->id, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "handle_l_put:: done.";
  LOG(INFO) << "handle_l_put:: rq_table=\n" << rq_table.to_str();
}
/********* handle ri_req *********/
void RIManager::handle_ri_req(char* ri_req)
{
  std::map<std::string, std::string> ri_req_map = imsg_coder.decode(ri_req);
  // LOG(INFO) << "handle_ri_req:: ri_req_map=";
  // print_str_map(ri_req_map);
  //
  int app_id = boost::lexical_cast<int>(ri_req_map["app_id"]);
  ri_bc_server_->reinit_listen_client(app_id);
  //
  boost::shared_ptr<BCClient> bc_client_(
    new BCClient(app_id, num_cnodes, RI_MAX_MSG_SIZE, "ri_reply_", ds_driver_)
  );
  appid_bcclient_map[app_id] = bc_client_;
  //
  std::string type = ri_req_map["type"];
  
  if (type.compare(REMOTE_GET) == 0){
    handle_r_get(app_id, ri_req_map);
  }
  else if (type.compare(REMOTE_PUT) == 0){
    //
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
  
}

//Dor now assuming r_get will only be used for remote data
//TODO: a global get which will return local or remote data based on where data is
void RIManager::handle_r_get(int app_id, std::map<std::string, std::string> r_get_map)
{
  LOG(INFO) << "handle_r_get:: r_get_map=";
  print_str_map(r_get_map);
  
  std::string key = r_get_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(r_get_map["ver"]);
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = REMOTE_GET_REPLY;
  msg_map["key"] = key;
  msg_map["ver"] = r_get_map["ver"];
  
  char ds_id;
  if (!rq_table.get_key_ver(key, ver, ds_id, NULL, NULL, NULL, NULL, NULL) ){
    if (!remote_query(key, ver) ){
      LOG(ERROR) << "handle_r_get:: remote_query failed!";
    }
    if (!rq_table.get_key_ver(key, ver, ds_id, NULL, NULL, NULL, NULL, NULL) ){
      ds_id = '?';
    }
  }
  
  if(ds_id == '?'){
    LOG(INFO) << "handle_r_get:: <key= " << key << ", ver= " << ver << "> does not exist.";
    msg_map["ds_id"] = '?';
    appid_bcclient_map[app_id]->send(msg_map);
    return;
  }
  
  LOG(INFO) << "handle_r_get:: key= " << key << " exists in ds_id= " << ds_id << ".";
  if (!remote_fetch(ds_id, r_get_map) ){
    LOG(INFO) << "handle_r_get:: remote_fetch failed!";
    msg_map["ds_id"] = '?';
    appid_bcclient_map[app_id]->send(msg_map);
    return;
  }
  
  msg_map["ds_id"] = ds_id;
  appid_bcclient_map[app_id]->send(msg_map);
  //
  LOG(INFO) << "handle_r_get:: done.";
}

//PI: a key cannot be produced in multiple dataspaces
bool RIManager::remote_query(std::string key, unsigned int ver)
{
  LOG(INFO) << "remote_query:: started;";
  
  std::map<std::string, std::string> r_q_map;
  r_q_map["type"] = REMOTE_QUERY;
  r_q_map["key"] = key;
  r_q_map["ver"] = boost::lexical_cast<std::string>(ver);
  
  if (broadcast_msg(RIMSG, r_q_map) ){
    LOG(ERROR) << "remote_query:: broadcast_msg failed!";
    return false;
  }
  
  rq_syncer.add_sync_point(std::make_pair(key, ver), dht_node_->get_num_peers() );
  rq_syncer.wait(std::make_pair(key, ver) );
  rq_syncer.del_sync_point(std::make_pair(key, ver) );
  
  LOG(INFO) << "remote_query:: done.";
  return true;
}

int RIManager::broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->broadcast_msg(msg_type, msg_map);
}

int RIManager::send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->send_msg(ds_id, msg_type, msg_map);
}

bool RIManager::remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map)
{
  LOG(INFO) << "remote_fetch:: started;";
  
  r_fetch_map["type"] = REMOTE_FETCH;
  
  std::string ib_lport = rf_manager_->get_ib_lport();
  std::string ib_laddr_str( (const char*) ib_laddr);
  r_fetch_map["ib_laddr"] = ib_laddr_str;
  r_fetch_map["ib_lport"] = ib_lport;
  if(send_msg(ds_id, RIMSG, r_fetch_map) ){
    LOG(ERROR) << "remote_fetch:: send_msg to to_id= " << ds_id << " failed!";
    return false;
  }
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if(imsg_coder.decode_i_msg(r_fetch_map, key, ver, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "remote_fetch:: decode_i_msg failed!";
    free(gdim_);
    free(lb_);
    free(ub_);
    return false;
  }
  
  if (!rf_manager_->receive_put(ib_laddr_str, ib_lport, 
                                r_fetch_map["data_type"], key, ver, size,
                                ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "remote_fetch:: rf_manager_->receive_put failed!";
    free(gdim_);
    free(lb_);
    free(ub_);
    return false;
  }
  free(gdim_);
  free(lb_);
  free(ub_);
  //
  LOG(INFO) << "remote_fetch:: done.";
  return true;
}

/********* handle wamsg *********/
void RIManager::handle_wamsg(std::map<std::string, std::string> wamsg_map)
{
  std::string type = wamsg_map["type"];
  
  if (type.compare(REMOTE_QUERY) == 0){
    handle_r_query(wamsg_map);
  }
  else if (type.compare(REMOTE_QUERY_REPLY) == 0){
    handle_rq_reply(wamsg_map);
  }
  else if (type.compare(REMOTE_FETCH) == 0){
    handle_r_fetch(wamsg_map);
  }
  else if (type.compare(REMOTE_RQTABLE) == 0){
    handle_r_rqtable(wamsg_map);
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
  
  //
  LOG(INFO) << "handle_wamsg:: done.";
}

void RIManager::handle_r_query(std::map<std::string, std::string> r_query_map)
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
  char ds_id;
  if(!rq_table.get_key_ver(key, ver, ds_id, NULL, NULL, NULL, NULL, NULL) ){
    //LOG(INFO) << "handle_r_query:: does not exist; key= " << key;
    rq_reply_map["ds_id"] = '?';
  }
  else{
    rq_reply_map["ds_id"] = ds_id;
  }
  
  //LOG(INFO) << "handle_r_query:: rq_reply_map=";
  //print_str_map(rq_reply_map);
  
  if(send_msg(to_id, RIMSG, rq_reply_map) ){
    LOG(ERROR) << "handle_r_query:: send_msg to to_id= " << to_id << " failed!";
  }
  
  // LOG(INFO) << "handle_r_query:: rq_reply_map=";
  // print_str_map(rq_reply_map);
  //
  LOG(INFO) << "handle_r_query:: done.";
}

void RIManager::handle_rq_reply(std::map<std::string, std::string> rq_reply_map)
{
  LOG(INFO) << "handle_rq_reply:: rq_reply_map=";
  print_str_map(rq_reply_map);
  
  std::string key = rq_reply_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(rq_reply_map["ver"]);
  char ds_id = rq_reply_map["ds_id"].c_str()[0];
  if (ds_id != '?'){
    rq_table.put_key_ver(key, ver, ds_id, 0, 0, NULL, NULL, NULL);
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
  if(imsg_coder.decode_i_msg(r_fetch_map, key, ver, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_r_fetch:: decode_i_msg failed!";
    free(gdim_);
    free(lb_);
    free(ub_);
    return;
  }
  
  if (!rf_manager_->get_send(r_fetch_map["data_type"], key, ver, size,
                             ndim, gdim_, lb_, ub_,
                             r_fetch_map["ib_laddr"].c_str(), r_fetch_map["ib_lport"].c_str() ) ){
    LOG(ERROR) << "handle_r_fetch:: rf_manager_->get_send failed!";
  }
  free(gdim_);
  free(lb_);
  free(ub_);
  //
  LOG(INFO) << "handle_r_fetch:: done.";
}

void RIManager::handle_r_rqtable(std::map<std::string, std::string> r_rqtable_map)
{
  LOG(INFO) << "handle_r_rqtable:: r_rqtable_map=";
  print_str_map(r_rqtable_map);
  
  int count = 0;
  while(1)
  {
    std::string tail_str = boost::lexical_cast<std::string>(count);
    std::string key_str = "key_" + tail_str;
    if (!r_rqtable_map.count(key_str) ){
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
    for (int i = 0; i < ndim; i++){
      gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_tokens_it);
      gdim_tokens_it++;
      lb_[i] = boost::lexical_cast<uint64_t>(*lb_tokens_it);
      lb_tokens_it++;
      ub_[i] = boost::lexical_cast<uint64_t>(*ub_tokens_it);
      ub_tokens_it++;
    }
    
    rq_table.put_key_ver(r_rqtable_map[key_str], boost::lexical_cast<unsigned int>(r_rqtable_map["ver_"+tail_str]),
                         r_rqtable_map["ds_id_"+tail_str].c_str()[0],
                         boost::lexical_cast<int>(r_rqtable_map["size_"+tail_str]),
                         ndim, gdim_, lb_, ub_ );
    free(gdim_);
    free(lb_);
    free(ub_);
    
    count++;
  }
  //
  LOG(INFO) << "handle_rq_reply:: done.";
}

