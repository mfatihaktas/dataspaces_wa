#include "sim.h"

bool return_true() { return true; }

/*******************************************  QTable  *********************************************/
QTable::QTable()
{
  // 
  LOG(INFO) << "QTable:: constructed.";
}

QTable::~QTable() { LOG(INFO) << "QTable:: destructed."; }

std::string QTable::to_str()
{
  std::stringstream ss;
  
  for (rtree_t::const_query_iterator it = rtree.qbegin(boost::geometry::index::satisfies(boost::bind(&return_true) ) ); it != rtree.qend(); ++it) {
    point_t lp = (it->first).min_corner();
    point_t up = (it->first).max_corner();
    int lcoor_[NDIM], ucoor_[NDIM];
    BOOST_PP_REPEAT(NDIM, POINT_GET_REP, (lp, lcoor_) )
    BOOST_PP_REPEAT(NDIM, POINT_GET_REP, (up, ucoor_) )
    
    ss << "\t box= [(";
    for (int i = 0; i < NDIM; i++, ss << ",")
      ss << lcoor_[i];
    ss << ") -- ";
    for (int i = 0; i < NDIM; i++, ss << ",")
      ss << ucoor_[i];
    ss << ")] : " << it->second << "\n";
  }
  
  return ss.str();
}

int QTable::add(int* lcoor_, int* ucoor_, char ds_id)
{
  IS_VALID_BOX("add", lcoor_, ucoor_, return 1)
  CREATE_BOX(b, lcoor_, ucoor_)
  
  rtree.insert(std::make_pair(b, ds_id) );
  
  return 0;
}

int QTable::query(int* lcoor_, int* ucoor_, std::vector<char>& ds_id_v)
{
  IS_VALID_BOX("query", lcoor_, ucoor_, return 1)
  CREATE_BOX(qb, lcoor_, ucoor_)
  
  std::vector<value> value_v;
  // rtree.query(boost::geometry::index::intersects(qb), std::back_inserter(value_v) );
  rtree.query(boost::geometry::index::contains(qb), std::back_inserter(value_v) );
  
  for (std::vector<value>::iterator it = value_v.begin(); it != value_v.end(); it++)
    ds_id_v.push_back(it->second);
  
  return 0;
}

/******************************************  WASpace  *********************************************/
WASpace::WASpace(std::vector<char> ds_id_v, int pbuffer_size,
                 int* lcoor_, int* ucoor_)
: ds_id_v(ds_id_v), pbuffer_size(pbuffer_size),
  lcoor_(lcoor_), ucoor_(ucoor_)
{
  IS_VALID_BOX("WASpace", lcoor_, ucoor_, exit(1) )
  // 
  LOG(INFO) << "WASpace:: constructed.";
}

WASpace::~WASpace() { LOG(INFO) << "WASpace:: destructed."; }

std::string WASpace::to_str()
{
  std::stringstream ss;
  
  ss << "ds_id_v= " << patch_sfc::vec_to_str<char>(ds_id_v) << "\n";
  ss << "pbuffer_size= " << pbuffer_size << "\n";
  ss << "qtable= \n" << qtable.to_str() << "\n";
  ss << "app_id__ds_id_map= \n" << patch_sfc::map_to_str<>(app_id__ds_id_map) << "\n";
  
  return ss.str();
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

int WASpace::put(int p_id, int* lcoor_, int* ucoor_)
{
  if (app_id__ds_id_map.count(p_id) == 0) {
    LOG(ERROR) << "put:: non-reged p_id= " << p_id;
    return 1;
  }
  if (qtable.add(lcoor_, ucoor_, app_id__ds_id_map[p_id] ) ) {
    LOG(ERROR) << "put:: qtable.add failed! for " << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n";
    return 1;
  }
  
  return 0;
}

int WASpace::get(int c_id, int* lcoor_, int* ucoor_, char& get_type)
{
  CREATE_BOX(acced_box, lcoor_, ucoor_)
  acced_box_v.push_back(acced_box);
  
  std::vector<char> ds_id_v;
  if (qtable.query(lcoor_, ucoor_, ds_id_v) )
    return 1;
  else {
    if (std::find(ds_id_v.begin(), ds_id_v.end(), app_id__ds_id_map[c_id] ) == ds_id_v.end() )
      get_type = 'r';
    else
      get_type = 'l';
  }
}

/*******************************************  PCSim  **********************************************/
PCSim::PCSim(std::vector<char> ds_id_v, int pbuffer_size,
             int* lcoor_, int* ucoor_,
             std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v)
: wa_space(ds_id_v, pbuffer_size, lcoor_, ucoor_)
{
  for (int p_id = 0; p_id < p_id__ds_id_v.size(); p_id++)
    wa_space.reg_app(p_id, p_id__ds_id_v[p_id] );
  
  for (int c_id = 0; c_id < c_id__ds_id_v.size(); c_id++) {
    wa_space.reg_app(p_id__ds_id_v.size() + c_id, c_id__ds_id_v[c_id] );
    
    std::vector<char> get_type_v;
    c_id__get_type_v_map[c_id] = get_type_v;
  }
  // 
  LOG(INFO) << "PCSim:: constructed.";
}

PCSim::~PCSim() { LOG(INFO) << "PCSim:: destructed."; }

std::string PCSim::to_str()
{
  std::stringstream ss;
  
  ss << "wa_space= \n" << wa_space.to_str() << "\n";
  
  return ss.str();
}

std::map<int, float> PCSim::get_c_id__get_lperc_map()
{
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_v_map.begin(); map_it != c_id__get_type_v_map.end(); map_it++) {
    int num_l = 0;
    for (std::vector<char>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      if (*vec_it == 'l')
        ++num_l;
    }
    
    c_id__get_lperc_map[map_it->first] = (float) num_l / (map_it->second).size();
  }
  
  return c_id__get_lperc_map;
}

void PCSim::sim(std::vector<app_id__lcoor_ucoor_pair>& p_id__lcoor_ucoor_pair_v,
                std::vector<app_id__lcoor_ucoor_pair>& c_id__lcoor_ucoor_pair_v)
{
  for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = p_id__lcoor_ucoor_pair_v.begin(); it != p_id__lcoor_ucoor_pair_v.end(); it++) {
    wa_space.put(it->first, (it->second).first, (it->second).second);
    LOG(INFO) << "sim:: put " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
  }
  
  for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = c_id__lcoor_ucoor_pair_v.begin(); it != c_id__lcoor_ucoor_pair_v.end(); it++) {
    char get_type;
    if (wa_space.get(it->first, (it->second).first, (it->second).second, get_type) )
      LOG(WARNING) << "sim:: could NOT get " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
    else {
      LOG(INFO) << "sim:: got " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
      c_id__get_type_v_map[it->first].push_back(get_type);
    }
  }
  
}
