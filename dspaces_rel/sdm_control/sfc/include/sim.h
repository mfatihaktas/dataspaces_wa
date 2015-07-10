#ifndef _SIM_H_
#define _SIM_H_

#include <math.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/preprocessor.hpp>
#include "boost/tuple/tuple.hpp"
#include <boost/assign.hpp>
#include "boost/icl/interval.hpp"
#include <boost/icl/interval_map.hpp>

#include <glog/logging.h>

#include "hilbert.h"
#include "patch_sfc.h"

#define NDIM 2

/*******************************************  QTable  *********************************************/
typedef boost::geometry::model::point<int, NDIM, boost::geometry::cs::cartesian> point_t;
typedef boost::geometry::model::box<point_t> box_t;
typedef std::pair<box_t, char> value;

class QTable {
  typedef boost::geometry::index::rtree<value, boost::geometry::index::quadratic<16> > rtree_t;
  private:
     rtree_t rtree;
     
  public:
    QTable();
    ~QTable();
    std::string to_str();
    
    int add(int* lcoor_, int* ucoor_, char ds_id);
    int query(int* lcoor_, int* ucoor_, std::vector<char>& ds_id_v);
};

/*****************************************  HPredictor  *******************************************/
typedef std::pair<int*, int*> lcoor_ucoor_pair;
class HPredictor { // Hilbert
  private:
    int *lcoor_, *ucoor_;
    
    int hilbert_num_bits;
    boost::icl::interval_map<bitmask_t, std::set<char> > index_interval__ds_id_set_map;
  public:
    HPredictor(int* lcoor_, int* ucoor_);
    ~HPredictor();
    std::string to_str();
    
    int add_acc__predict_next_acc(char ds_id, int* lcoor_, int* ucoor_,
                                  std::vector<lcoor_ucoor_pair>& next_lcoor_ucoor_pair_v);
};

/******************************************  WASpace  *********************************************/
class WASpace {
  private:
    std::vector<char> ds_id_v;
    int pbuffer_size;
    int *lcoor_, *ucoor_;
    
    QTable qtable;
    
    std::vector<box_t> acced_box_v;
    
    std::map<int, char> app_id__ds_id_map;
    std::vector<box_t> content_v;
    std::map<char, boost::shared_ptr<std::vector<box_t> > > ds_id__content_v_map;
  public:
    WASpace(std::vector<char> ds_id_v, int pbuffer_size,
            int* lcoor_, int* ucoor_);
    ~WASpace();
    std::string to_str();
    
    int reg_app(int app_id, char ds_id);
    
    int put(int p_id, int* lcoor_, int* ucoor_);
    int get(int c_id, int* lcoor_, int* ucoor_, char& get_type);
};

/*******************************************  PCSim  **********************************************/
typedef std::pair<int, lcoor_ucoor_pair> app_id__lcoor_ucoor_pair;

class PCSim { // Prefetching Simulator
  private:
    WASpace wa_space;
    
    std::map<int, std::vector<char> > c_id__get_type_v_map;
    std::map<int, float> c_id__get_lperc_map;
    
  public:
    PCSim(std::vector<char> ds_id_v, int pbuffer_size,
          int* lcoor_, int* ucoor_,
          std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v);
    ~PCSim();
    std::string to_str();
    std::map<int, float> get_c_id__get_lperc_map();
    
    void sim(std::vector<app_id__lcoor_ucoor_pair>& p_id__lcoor_ucoor_pair_v,
             std::vector<app_id__lcoor_ucoor_pair>& c_id__lcoor_ucoor_pair_v);
};

#endif // _SIM_H_
