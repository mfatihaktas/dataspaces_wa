#include "sfc.h"

/********************************************  SAlgo  *********************************************/
SAlgo::SAlgo(COOR_T* lcoor_, COOR_T* ucoor_)
: lcoor_(lcoor_), ucoor_(ucoor_)
{
  IS_VALID_BOX("SAlgo", lcoor_, ucoor_, exit(1) )
  
  std::vector<int> dim_length_v;
  for (int i = 0; i < NDIM; i++)
    dim_length_v.push_back(ucoor_[i] - lcoor_[i] );
  
  hilbert_num_bits = (int) ceil(log2(*std::max_element(dim_length_v.begin(), dim_length_v.end() ) ) );
  max_index = pow(2, NDIM*hilbert_num_bits) - 1;
  // 
  LOG(INFO) << "SAlgo:: constructed.";
}

std::string SAlgo::to_str()
{
  std::stringstream ss;
  ss << "\t lcoor_= " << patch_all::arr_to_str<>(NDIM, lcoor_) << "\n"
     << "\t ucoor_= " << patch_all::arr_to_str<>(NDIM, ucoor_) << "\n";
  // ss << "\t index_interval__ds_id_set_map= \n";
  // for (index_interval__ds_id_set_map_t::iterator it = index_interval__ds_id_set_map.begin(); it != index_interval__ds_id_set_map.end(); it++)
  //   ss << "\t" << it->first << " : " << patch_all::set_to_str<>(it->second) << "\n";
  
  return ss.str();
}

boost::shared_ptr<index_interval_set_t> SAlgo::coor_to_index_interval_set_(COOR_T* lcoor_, COOR_T* ucoor_)
{
  boost::shared_ptr<index_interval_set_t> index_interval_set_ = boost::make_shared<index_interval_set_t>();
  MULTI_FOR(lcoor_, ucoor_)
    bitmask_t walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    bitmask_t index = mfa_hilbert_c2i(NDIM, hilbert_num_bits, walk_lcoor_);
    
    index_interval_set_->insert(boost::icl::interval<bitmask_t>::closed(index, index) );
  END_MULTI_FOR()
  
  return index_interval_set_;
}

void SAlgo::index_interval_set_to_coor_v(index_interval_set_t interval_set, std::vector<COOR_T*>& coor_v)
{
  for (index_interval_set_t::iterator it = interval_set.begin(); it != interval_set.end(); it++) {
    for (bitmask_t index = it->lower(); index <= it->upper(); index++) {
      bitmask_t* coor_ = (bitmask_t*)malloc(NDIM*sizeof(bitmask_t) );
      // std::cout << "index_interval_set_to_coor_v:: index= " << index << "\n";
      mfa_hilbert_i2c(NDIM, hilbert_num_bits, index, coor_);
      // TODO: Why ?
      COOR_T* good_t_coor_ = (COOR_T*)malloc(NDIM*sizeof(int) );
      for (int i = 0; i < NDIM; i++)
        good_t_coor_[i] = (COOR_T) coor_[i];
      
      free(coor_);
      
      coor_v.push_back(good_t_coor_);
    }
  }
}

// Expands interval_set by expand_length from both sides
void SAlgo::expand_interval_set(bitmask_t expand_length, index_interval_set_t& ii_set)
{
  index_interval_set_t _ii_set;
  for (index_interval_set_t::iterator it = ii_set.begin(); it != ii_set.end(); it++) {
    bitmask_t lower = it->lower() - expand_length;
    if (it->lower() < expand_length) // bitmask_t > 0
      lower = 0;
    
    _ii_set.insert(boost::icl::interval<bitmask_t>::closed(lower, std::min<bitmask_t>(max_index, it->upper() + expand_length) ) );
  }
  ii_set = _ii_set;
}

int SAlgo::add_access(COOR_T* lcoor_, COOR_T* ucoor_)
{
  boost::shared_ptr<index_interval_set_t> index_interval_set_ = coor_to_index_interval_set_(lcoor_, ucoor_);
  acced_index_interval_set_v.push_back(index_interval_set_);
  
  return 0;
}

/*******************************************  MHSAlgo  ********************************************/

/********************************************  HSAlgo  ********************************************/
HSAlgo::HSAlgo(COOR_T* lcoor_, COOR_T* ucoor_,
               bitmask_t expand_length)
: SAlgo(lcoor_, ucoor_),
  expand_length(expand_length)
{
  // 
  LOG(INFO) << "HSAlgo:: constructed.";
}

std::string HSAlgo::to_str()
{
  std::stringstream ss;
  ss << "hilbert_num_bits= " << hilbert_num_bits << "\n"
     << "max_index= " << max_index << "\n"
     << "SAlgo::to_str= " << SAlgo::to_str();
  
  return ss.str();
}

// int HSAlgo::put(int ds_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
// {
//   boost::shared_ptr<index_interval_set_t> index_interval_set_ = coor_to_index_interval_set_(lcoor_, ucoor_);
  
//   std::set<int> ds_id_set;
//   ds_id_set.insert(ds_id);
//   for (index_interval_set_t::iterator it = index_interval_set_->begin(); it != index_interval_set_->end(); it++)
//     index_interval__ds_id_set_map += std::make_pair(*it, ds_id_set);
  
//   return 0;
// }

int HSAlgo::get_to_fetch(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<lcoor_ucoor_pair>& lucoor_to_fetch_v)
{
  // Using the last acced index_interval_set predict from locality
  index_interval_set_t& ii_set = *(acced_index_interval_set_v.back() );
  
  LOG(INFO) << "get_to_prefetch:: " << LUCOOR_TO_STR(lcoor_, ucoor_) << ", ii_set= " << ii_set << "\n";
  expand_interval_set(expand_length, ii_set);
  LOG(INFO) << "get_to_prefetch:: after expand_interval_set= " << ii_set << "\n";
  
  RTable<int> rtable;
  
  std::vector<COOR_T*> lcoor_v;
  index_interval_set_to_coor_v(ii_set, lcoor_v);
  for (std::vector<COOR_T*>::iterator lcoor__ = lcoor_v.begin(); lcoor__ != lcoor_v.end(); lcoor__++) {
    COOR_T* ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    for (int i = 0; i < NDIM; i++)
      ucoor_[i] = (*lcoor__)[i] + 1;

    // lucoor_to_fetch_v.push_back(std::make_pair(*lcoor__, ucoor_) );
    rtable.add("", 0, *lcoor__, ucoor_, -1);
  }
  
  COOR_T* to_fetch_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  COOR_T* to_fetch_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  rtable.get_bound_lucoor(to_fetch_lcoor_, to_fetch_ucoor_);
  lucoor_to_fetch_v.push_back(std::make_pair(to_fetch_lcoor_, to_fetch_ucoor_) );
}
