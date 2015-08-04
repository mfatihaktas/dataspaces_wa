#ifndef _SFC_H_
#define _SFC_H_

#include <math.h>
#include <algorithm>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/preprocessor.hpp>
#include "boost/tuple/tuple.hpp"
#include <boost/assign.hpp>
#include "boost/icl/interval.hpp"
#include <boost/icl/interval_map.hpp>
#include <boost/lambda/lambda.hpp>

#include <glog/logging.h>

#include "hilbert.h"
#include "patch_sfc.h"

#define NDIM 2

/*******************************************  RTable  *********************************************/
typedef uint64_t COOR_T;
typedef boost::geometry::model::point<int, NDIM, boost::geometry::cs::cartesian> point_t;
typedef boost::geometry::model::box<point_t> box_t;

template <typename VAL_T>
class RTable {
  typedef std::pair<box_t, VAL_T> value;
  typedef boost::geometry::index::rtree<value, boost::geometry::index::quadratic<16> > rtree_t;
  
  private:
    rtree_t rtree;
    
  public:
    RTable() { LOG(INFO) << "RTable:: constructed."; }
    ~RTable() { LOG(INFO) << "RTable:: destructed."; }
    std::string to_str()
    {
      std::stringstream ss;
      for (typename rtree_t::const_query_iterator it = rtree.qbegin(boost::geometry::index::satisfies(boost::lambda::constant(true) ) ); it != rtree.qend(); ++it) {
        ss << "\t box= " << boost::geometry::dsv(it->first) << " : " << it->second << "\n";
        // point_t lp = (it->first).min_corner();
        // point_t up = (it->first).max_corner();
        // int lcoor_[NDIM], ucoor_[NDIM];
        // BOOST_PP_REPEAT(NDIM, POINT_GET_REP, (lp, lcoor_) )
        // BOOST_PP_REPEAT(NDIM, POINT_GET_REP, (up, ucoor_) )
        
        // ss << "\t box= [(";
        // for (int i = 0; i < NDIM; i++, ss << ",")
        //   ss << lcoor_[i];
        // ss << ") -- ";
        // for (int i = 0; i < NDIM; i++, ss << ",")
        //   ss << ucoor_[i];
        // ss << ")] : " << it->second << "\n";
      }
      
      return ss.str();
    }
    
    int add(COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val)
    {
      IS_VALID_BOX("add", lcoor_, ucoor_, return 1)
      CREATE_BOX(0, b, lcoor_, ucoor_)
      
      rtree.insert(std::make_pair(b, val) );
      
      return 0;
    }
    
    int query(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v)
    {
      IS_VALID_BOX("query", lcoor_, ucoor_, return 1)
      CREATE_BOX(0, qb, lcoor_, ucoor_)
      
      std::vector<value> value_v;
      // rtree.query(boost::geometry::index::intersects(qb), std::back_inserter(value_v) );
      rtree.query(boost::geometry::index::contains(qb), std::back_inserter(value_v) );
      
      for (typename std::vector<value>::iterator it = value_v.begin(); it != value_v.end(); it++)
        val_v.push_back(it->second);
      
      return (val_v.size() == 0);
    }
};

/*****************************************  MPredictor  *******************************************/

class MPredictor { // Markov; tries to learn on 1d hilbert index
  
};

/*****************************************  HPredictor  *******************************************/
typedef std::pair<COOR_T*, COOR_T*> lcoor_ucoor_pair;
typedef boost::icl::interval_set<bitmask_t> index_interval_set_t;
typedef boost::icl::interval_map<bitmask_t, std::set<char> > index_interval__ds_id_set_map_t;
class HPredictor { // Hilbert
  private:
    COOR_T *lcoor_, *ucoor_;
    
    int hilbert_num_bits, max_index;
    std::vector<boost::shared_ptr<index_interval_set_t> > acced_index_interval_set_v;
    index_interval__ds_id_set_map_t index_interval__ds_id_set_map;
  public:
    HPredictor(COOR_T* lcoor_, COOR_T* ucoor_);
    ~HPredictor();
    std::string to_str();
    
    boost::shared_ptr<index_interval_set_t> coor_to_index_interval_set_(COOR_T* lcoor_, COOR_T* ucoor_);
    void index_interval_set_to_coor_v(index_interval_set_t interval_set, std::vector<COOR_T*>& coor_v);
    void expand_1_interval_set(bitmask_t expand_length, index_interval_set_t& interval_set);
    
    int put(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_);
    int add_acc__predict_next_acc(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_,
                                  bitmask_t expand_length, std::vector<lcoor_ucoor_pair>& next_lcoor_ucoor_pair_v);
};

/******************************************  WASpace  *********************************************/
class WASpace {
  private:
    std::vector<char> ds_id_v;
    int pbuffer_size, pexpand_length;
    COOR_T *lcoor_, *ucoor_;
    
    RTable<char> rtable;
    
    std::map<int, char> app_id__ds_id_map;
    std::vector<box_t> content_v;
    std::map<char, boost::shared_ptr<std::vector<box_t> > > ds_id__content_v_map;
    
    std::vector<box_t> acced_box_v;
    HPredictor hpredictor;
  public:
    WASpace(std::vector<char> ds_id_v,
            int pbuffer_size, int pexpand_length,
            COOR_T* lcoor_, COOR_T* ucoor_);
    ~WASpace();
    std::string to_str();
    
    int reg_ds(char ds_id);
    int reg_app(int app_id, char ds_id);
    int prefetch(char ds_id, COOR_T* lcoor_, COOR_T* ucoor_);
    
    int put(int p_id, COOR_T* lcoor_, COOR_T* ucoor_);
    int query(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char> ds_id_v);
    int get(int c_id, COOR_T* lcoor_, COOR_T* ucoor_, char& get_type,
            std::vector<lcoor_ucoor_pair>& next_lcoor_ucoor_pair_v);
};

#endif // _SFC_H_
