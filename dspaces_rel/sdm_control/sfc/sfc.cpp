#include "sfc.h"

/*****************************************  MPredictor  *******************************************/


/*****************************************  HPredictor  *******************************************/
HPredictor::HPredictor(COOR_T* lcoor_, COOR_T* ucoor_)
: lcoor_(lcoor_), ucoor_(ucoor_)
{
  IS_VALID_BOX("HPredictor", lcoor_, ucoor_, exit(1) )
  // 
  std::vector<int> dim_length_v;
  for (int i = 0; i < NDIM; i++)
    dim_length_v.push_back(ucoor_[i] - lcoor_[i] );
  
  hilbert_num_bits = (int) ceil(log2(*std::max_element(dim_length_v.begin(), dim_length_v.end() ) ) );
  max_index = pow(2, NDIM*hilbert_num_bits) - 1;
  // 
  LOG(INFO) << "HPredictor:: constructed.";
}

HPredictor::~HPredictor() { LOG(INFO) << "HPredictor:: destructed."; }

std::string HPredictor::to_str()
{
  std::stringstream ss;
  ss << "\t lcoor_= " << patch_sfc::arr_to_str<>(NDIM, lcoor_) << "\n"
     << "\t ucoor_= " << patch_sfc::arr_to_str<>(NDIM, ucoor_) << "\n"
     << "\t hilbert_num_bits= " << hilbert_num_bits << "\n"
     << "\t max_index= " << max_index << "\n";
  
  ss << "\t index_interval__ds_id_set_map= \n";
  for (index_interval__ds_id_set_map_t::iterator it = index_interval__ds_id_set_map.begin(); it != index_interval__ds_id_set_map.end(); it++)
    ss << "\t" << it->first << " : " << patch_sfc::set_to_str<>(it->second) << "\n";
  
  return ss.str();
}

boost::shared_ptr<index_interval_set_t>
HPredictor::coor_to_index_interval_set_(COOR_T* lcoor_, COOR_T* ucoor_)
{
  boost::shared_ptr<index_interval_set_t> index_interval_set_ = boost::make_shared<index_interval_set_t>();
  MULTI_FOR(lcoor_, ucoor_)
    bitmask_t walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    bitmask_t index = hilbert_c2i(NDIM, hilbert_num_bits, walk_lcoor_);
    
    index_interval_set_->insert(boost::icl::interval<bitmask_t>::closed(index, index) );
  END_MULTI_FOR()
  
  return index_interval_set_;
}

void HPredictor::index_interval_set_to_coor_v(index_interval_set_t interval_set, std::vector<COOR_T*>& coor_v)
{
  for (index_interval_set_t::iterator it = interval_set.begin(); it != interval_set.end(); it++) {
    for (bitmask_t index = it->lower(); index <= it->upper(); index++) {
      bitmask_t* coor_ = (bitmask_t*)malloc(NDIM*sizeof(bitmask_t) );
      // std::cout << "index_interval_set_to_coor_v:: index= " << index << "\n";
      hilbert_i2c(NDIM, hilbert_num_bits, index, coor_);
      
      COOR_T* good_t_coor_ = (COOR_T*)malloc(NDIM*sizeof(int) );
      for (int i = 0; i < NDIM; i++)
        good_t_coor_[i] = (COOR_T) coor_[i];
      
      free(coor_);
      
      coor_v.push_back(good_t_coor_);
    }
  }
}

// Expands interval_set by expand_length from both sides
void HPredictor::expand_1_interval_set(bitmask_t expand_length, index_interval_set_t& interval_set)
{
  index_interval_set_t i_set;
  for (index_interval_set_t::iterator it = interval_set.begin(); it != interval_set.end(); it++) {
    bitmask_t lower = it->lower() - expand_length;
    if (it->lower() < expand_length)
      lower = 0;
    
    i_set.insert(boost::icl::interval<bitmask_t>::closed(lower, std::min<bitmask_t>(max_index, it->upper() + expand_length) ) );
  }
  interval_set = i_set;
}

int HPredictor::put(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_)
{
  boost::shared_ptr<index_interval_set_t> index_interval_set_ = coor_to_index_interval_set_(lcoor_, ucoor_);
  
  std::set<char> ds_id_set;
  ds_id_set.insert(ds_id);
  for (index_interval_set_t::iterator it = index_interval_set_->begin(); it != index_interval_set_->end(); it++)
    index_interval__ds_id_set_map += std::make_pair(*it, ds_id_set);
  
  return 0;
}

int HPredictor::add_acc__predict_next_acc(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_,
                                          bitmask_t expand_length, std::vector<lcoor_ucoor_pair>& next_lcoor_ucoor_pair_v)
{
  boost::shared_ptr<index_interval_set_t> index_interval_set_ = coor_to_index_interval_set_(lcoor_, ucoor_);
  acced_index_interval_set_v.push_back(index_interval_set_);
  // predict
  // with locality
  // std::cout << "add_acc__predict_next_acc:: " << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n";
  // std::cout << "add_acc__predict_next_acc:: index_interval_set= " << *index_interval_set_ << "\n";
  expand_1_interval_set(expand_length, *index_interval_set_);
  // std::cout << "add_acc__predict_next_acc:: after expand_1 index_interval_set= " << *index_interval_set_ << "\n";
  
  std::vector<COOR_T*> lcoor_v;
  index_interval_set_to_coor_v(*index_interval_set_, lcoor_v);
  for (std::vector<COOR_T*>::iterator next_lcoor__ = lcoor_v.begin(); next_lcoor__ != lcoor_v.end(); next_lcoor__++) {
    COOR_T* next_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    for (int i = 0; i < NDIM; i++)
      next_ucoor_[i] = (*next_lcoor__)[i] + 1;
    
    next_lcoor_ucoor_pair_v.push_back(std::make_pair(*next_lcoor__, next_ucoor_) );
  }
}

/******************************************  WASpace  *********************************************/
WASpace::WASpace(std::vector<char> ds_id_v, 
                 int pbuffer_size, int pexpand_length,
                 COOR_T* lcoor_, COOR_T* ucoor_)
: ds_id_v(ds_id_v),
  pbuffer_size(pbuffer_size), pexpand_length(pexpand_length),
  lcoor_(lcoor_), ucoor_(ucoor_),
  hpredictor(lcoor_, ucoor_)
{
  IS_VALID_BOX("WASpace", lcoor_, ucoor_, exit(1) )
  // 
  LOG(INFO) << "WASpace:: constructed.";
}

WASpace::~WASpace() { LOG(INFO) << "WASpace:: destructed."; }

std::string WASpace::to_str()
{
  std::stringstream ss;
  
  ss << "ds_id_v= " << patch_sfc::vec_to_str<char>(ds_id_v) << "\n"
     << "pbuffer_size= " << pbuffer_size << "\n"
     << "pexpand_length= " << pexpand_length << "\n"
     << "app_id__ds_id_map= \n" << patch_sfc::map_to_str<>(app_id__ds_id_map) << "\n"
     << "pexpand_length= " << pexpand_length << "\n"
     << "hpredictor= \n" << hpredictor.to_str() << "\n";
    // << "rtable= \n" << rtable.to_str() << "\n";
  
  return ss.str();
}

int WASpace::reg_ds(char ds_id)
{
  if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() ) {
    LOG(WARNING) << "reg_ds:: ds_id= " << ds_id << " is already reged!";
    return 1;
  }
  ds_id_v.push_back(ds_id);
  
  return 0;
}

int WASpace::reg_app(int app_id, char ds_id)
{
  if (app_id__ds_id_map.count(app_id) != 0) {
    LOG(ERROR) << "reg_app:: already reged app_id= " << app_id;
    return 1;
  }
  app_id__ds_id_map[app_id] = ds_id;
  return 0;
}

int WASpace::prefetch(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_)
{
  IS_VALID_BOX("prefetch", lcoor_, ucoor_, return 1)
  CREATE_BOX(0, pre_box, lcoor_, ucoor_)
  
  std::vector<char> ds_id_v;
  if (rtable.query(lcoor_, ucoor_, ds_id_v) ) {
    // LOG(WARNING) << "prefetch:: not-put, cannot prefetch; " << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n";
    return 1;
  }
  else {
    if (rtable.add(lcoor_, ucoor_, ds_id) ) {
      LOG(ERROR) << "prefetch:: rtable.add failed! for " << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n";
      return 1;
    }
    return 0;
  }
}

int WASpace::put(int p_id, COOR_T* lcoor_, COOR_T* ucoor_)
{
  IS_VALID_BOX("put", lcoor_, ucoor_, return 1)
  
  if (app_id__ds_id_map.count(p_id) == 0) {
    LOG(ERROR) << "put:: non-reged p_id= " << p_id;
    return 1;
  }
  if (rtable.add(lcoor_, ucoor_, app_id__ds_id_map[p_id] ) ) {
    LOG(ERROR) << "put:: rtable.add failed! for " << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n";
    return 1;
  }
  
  return 0;
}

int WASpace::query(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char> ds_id_v) { return rtable.query(lcoor_, ucoor_, ds_id_v); }

int WASpace::get(int c_id, COOR_T* lcoor_, COOR_T* ucoor_, char& get_type,
                 std::vector<lcoor_ucoor_pair>& next_lcoor_ucoor_pair_v)
{
  IS_VALID_BOX("get", lcoor_, ucoor_, return 1)
  CREATE_BOX(0, acced_box, lcoor_, ucoor_)
  char ds_id = app_id__ds_id_map[c_id];
  
  std::vector<char> ds_id_v;
  if (rtable.query(lcoor_, ucoor_, ds_id_v) )
    return 1;
  else {
    if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) == ds_id_v.end() )
      get_type = 'r';
    else
      get_type = 'l';
  }
  
  acced_box_v.push_back(acced_box);
  hpredictor.add_acc__predict_next_acc(ds_id, lcoor_, ucoor_,
                                       (bitmask_t)pexpand_length, next_lcoor_ucoor_pair_v);
  
  return 0;
}
