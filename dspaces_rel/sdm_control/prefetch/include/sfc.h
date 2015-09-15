#ifndef _SFC_H_
#define _SFC_H_

#include <math.h>
#include <algorithm>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
// #include <boost/geometry/geometries/point_xy.hpp>
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
#include "patch_pre.h"

/******************************************  MACROS  **********************************************/
// Note: Most of them are used for tackling ugly boost::geometry interface for multi-dimensional case
#define POINT_GET_REP(z, n, p_coor) \
  BOOST_PP_TUPLE_ELEM(2, 1, p_coor)[n] = boost::geometry::get<n>(BOOST_PP_TUPLE_ELEM(2, 0, p_coor) );

#define POINT_SET_REP(z, n, p_coor) \
  boost::geometry::set<n>(BOOST_PP_TUPLE_ELEM(2, 0, p_coor), BOOST_PP_TUPLE_ELEM(2, 1, p_coor)[n] );

#define CREATE_BOX(i, box, lcoor_, ucoor_) \
  point_t lp ## i, up ## i; \
  BOOST_PP_REPEAT(NDIM, POINT_SET_REP, (lp ## i, lcoor_) ) \
  BOOST_PP_REPEAT(NDIM, POINT_SET_REP, (up ## i, ucoor_) ) \
  box_t box(lp ## i, up ## i);

#define KV_TO_STR(key, ver) \
  "<key= " << key << ", ver= " << ver << ">"

#define LUCOOR_TO_STR(lcoor_, ucoor_) \
  "lcoor_= " << patch_sfc::arr_to_str<>(NDIM, lcoor_) \
  << ", ucoor_= " << patch_sfc::arr_to_str<>(NDIM, ucoor_)

#define KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) \
  KV_TO_STR(key, ver) << ", " << LUCOOR_TO_STR(lcoor_, ucoor_)

#define IS_VALID_BOX(func_name, lcoor_, ucoor_, action) \
  for (int i = 0; i < NDIM; i++) { \
    if (ucoor_[i] <= lcoor_[i] ) { \
      LOG(ERROR) << #func_name":: not valid box; \n" << LUCOOR_TO_STR(lcoor_, ucoor_) << "\n"; \
      action; \
    } \
  }
// MULTI_FOR
#define VAR(d, n) BOOST_PP_CAT(d, n)
#define VAR_REP(z, n, d) d ## n
#define FOR_I(n) BOOST_PP_SUB(BOOST_PP_SUB(NDIM, n), 1)

#define FOR_REP(z, n, ll_ul_) \
  for(int VAR(d, n) = BOOST_PP_TUPLE_ELEM(2, 0, ll_ul_)[n]; VAR(d, n) < BOOST_PP_TUPLE_ELEM(2, 1, ll_ul_)[n]; VAR(d, n)++) {
  // for(int VAR(d, FOR_I(n) ) = BOOST_PP_TUPLE_ELEM(2, 0, ll_ul_)[FOR_I(n) ]; \
  //     VAR(d, FOR_I(n) ) < BOOST_PP_TUPLE_ELEM(2, 1, ll_ul_)[FOR_I(n) ]; VAR(d, FOR_I(n) )++) {
#define MULTI_FOR(ll_, ul_) BOOST_PP_REPEAT(NDIM, FOR_REP, (ll_, ul_) )

#define END_FOR_REP(z, n, data) }
#define END_MULTI_FOR() BOOST_PP_REPEAT(NDIM, END_FOR_REP, ~)

#define FIXED_REP(z, n, d) d

#define ARR_TO_ARG_LIST_REP(z, n, arr_) arr_[n]
// ---------------------------------------------------------------------------------------------- //
#define NDIM 1
typedef uint64_t COOR_T;

/********************************************  QTable  ********************************************/
template <typename VAL_T>
class QTable { // Query
  private:
    
  public:
    QTable() {};
    ~QTable() {};
    
    virtual std::string to_str() = 0;
    virtual int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val) = 0;
    virtual int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v) = 0;
};

/*******************************************  RTable  *********************************************/
typedef boost::geometry::model::point<int, NDIM, boost::geometry::cs::cartesian> point_t;
typedef boost::geometry::model::box<point_t> box_t;

// std::string point_to_str(point_t p)
// {
//   std::stringstream ss;
  
//   int coor_[NDIM];
//   BOOST_PP_REPEAT(NDIM, POINT_GET_REP, (p, coor_) );
//   ss << patch_sfc::arr_to_str(NDIM, coor_);
  
//   return ss.str();
// }

template <typename VAL_T>
class RTable : public QTable<VAL_T> {
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
    
    int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val)
    {
      IS_VALID_BOX("add", lcoor_, ucoor_, return 1)
      CREATE_BOX(0, b, lcoor_, ucoor_)
      
      rtree.insert(std::make_pair(b, val) );
      
      return 0;
    }
    
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v)
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

/*******************************************  KVTable  *********************************************/
typedef std::pair<std::string, unsigned int> key_ver_pair;

template <typename VAL_T>
class KVTable : public QTable<VAL_T> { // Key Ver
  private:
    std::map<key_ver_pair, VAL_T> kv_val_map;
    // patch_sdm::thread_safe_map<key_ver_pair, VAL_T> kv_val_map;
  public:
    KVTable() { LOG(INFO) << "KVTable:: constructed."; }
    ~KVTable() { LOG(INFO) << "KVTable:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "kv_val_map= \n" << patch_sfc::map_to_str<>(kv_val_map);
      return ss.str();
    }
    
    int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val)
    {
      IS_VALID_BOX("add", lcoor_, ucoor_, return 1)
      key_ver_pair kv = std::make_pair(key, ver);
      if (kv_val_map.contains(kv) ) {
        LOG(ERROR) << "add:: already contains; <key= " << key << ", ver= " << ver << ">";
        return 1;
      }
      
      kv_val_map[kv] = val;
      return 0;
    }
    
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v)
    {
      IS_VALID_BOX("query", lcoor_, ucoor_, return 1)
      key_ver_pair kv = std::make_pair(key, ver);
      if (!kv_val_map.contains(kv) )
        return 1;
      
      val_v.push_back(kv_val_map[kv] );
      
      return 0;
    }
};

/********************************************  SAlgo  *********************************************/
typedef std::pair<COOR_T*, COOR_T*> lcoor_ucoor_pair;
typedef std::pair<key_ver_pair, lcoor_ucoor_pair> kv_lucoor_pair;

typedef boost::icl::interval_set<bitmask_t> index_interval_set_t;
// typedef boost::icl::interval_map<bitmask_t, std::set<char> > index_interval__ds_id_set_map_t;

class SAlgo {
  protected:
    std::vector<boost::shared_ptr<index_interval_set_t> > acced_index_interval_set_v;
    // index_interval__ds_id_set_map_t index_interval__ds_id_set_map;
  public:
    SAlgo(COOR_T *lcoor_, *ucoor_);
    ~SAlgo();
    virtual std::string to_str();
    
    boost::shared_ptr<index_interval_set_t> coor_to_index_interval_set_(COOR_T* lcoor_, COOR_T* ucoor_);
    void index_interval_set_to_coor_v(index_interval_set_t interval_set, std::vector<COOR_T*>& coor_v);
    void expand_interval_set(bitmask_t expand_length, index_interval_set_t& interval_set);
    
    virtual int add_access(COOR_T* lcoor_, COOR_T* ucoor_);
    virtual int get_to_prefetch() = 0;
};

/*****************************************  MHSAlgo  ******************************************/
// class MHSAlgo { // Markov Hilbert; tries to learn on 1d hilbert index
// };

/*****************************************  HSAlgo  *******************************************/
class HSAlgo : public SAlgo { // Hilbert
  private:
    bitmask_t expand_length;
    
    int hilbert_num_bits, max_index;
  public:
    HSAlgo(COOR_T* lcoor_, COOR_T* ucoor_,
           bitmask_t expand_length);
    ~HSAlgo();
    std::string to_str();
    
    int get_to_prefetch();
};

#endif // _SFC_H_