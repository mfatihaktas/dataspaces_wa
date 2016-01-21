#ifndef _SFC_H_
#define _SFC_H_

#include <math.h>
#include <algorithm>
#include <functional>

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

#define NDIM 1
typedef uint64_t COOR_T;
typedef boost::geometry::model::point<int, NDIM, boost::geometry::cs::cartesian> point_t;
typedef boost::geometry::model::box<point_t> box_t;

/******************************************  MACROS  **********************************************/
// After \ there should not be any space
// Note: Most of them are used for tackling ugly boost::geometry interface for multi-dimensional case
#define POINT_GET_REP(z, n, p_coor) \
  BOOST_PP_TUPLE_ELEM(2, 1, p_coor)[n] = boost::geometry::get<n>(BOOST_PP_TUPLE_ELEM(2, 0, p_coor) );

#define POINT_SET_REP(z, n, p_coor) \
  boost::geometry::set<n>(BOOST_PP_TUPLE_ELEM(2, 0, p_coor), BOOST_PP_TUPLE_ELEM(2, 1, p_coor)[n] );

// corner: min, max
#define BOX_GET_CORNER_REP(z, n, box_corner_coor) \
  BOOST_PP_TUPLE_ELEM(3, 2, box_corner_coor)[n] = boost::geometry::get<boost::geometry::BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(3, 1, box_corner_coor), _corner), n>(BOOST_PP_TUPLE_ELEM(3, 0, box_corner_coor) );

// #define BOX_GET_CORNER_REP(z, n, box_coor) \
  // BOOST_PP_TUPLE_ELEM(2, 1, box_coor)[n] = boost::geometry::get<boost::geometry::max_corner, n>(BOOST_PP_TUPLE_ELEM(2, 0, box_coor) );

#define CREATE_BOX(i, box, lcoor_, ucoor_) \
  point_t lp ## i, up ## i; \
  BOOST_PP_REPEAT(NDIM, POINT_SET_REP, (lp ## i, lcoor_) ) \
  BOOST_PP_REPEAT(NDIM, POINT_SET_REP, (up ## i, ucoor_) ) \
  box_t box ## i(lp ## i, up ## i);

#define KV_TO_STR(key, ver) "<key= " << key << ", ver= " << ver << ">"

#define LUCOOR_TO_STR(lcoor_, ucoor_) \
  "lcoor_= " << patch::arr_to_str<>(NDIM, lcoor_) \
  << ", ucoor_= " << patch::arr_to_str<>(NDIM, ucoor_)

#define KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) \
  KV_TO_STR(key, ver) << ", " << LUCOOR_TO_STR(lcoor_, ucoor_)

#define IS_VALID_BOX(lcoor_, ucoor_, action) \
  for (int i = 0; i < NDIM; i++) { \
    if (ucoor_[i] <= lcoor_[i] ) { \
      log_(ERROR, "not valid box; \n" << LUCOOR_TO_STR(lcoor_, ucoor_) ) \
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
#define FOR_REP_W_INC(z, n, ll_ul_inc) \
  for(int VAR(d, n) = BOOST_PP_TUPLE_ELEM(3, 0, ll_ul_inc)[n]; VAR(d, n) < BOOST_PP_TUPLE_ELEM(3, 1, ll_ul_inc)[n]; VAR(d, n) += BOOST_PP_TUPLE_ELEM(3, 2, ll_ul_inc) ) {

#define MULTI_FOR(ll_, ul_) BOOST_PP_REPEAT(NDIM, FOR_REP, (ll_, ul_) )

#define MULTI_FOR_W_INC(ll_, ul_, inc) BOOST_PP_REPEAT(NDIM, FOR_REP_W_INC, (ll_, ul_, inc) )

#define END_FOR_REP(z, n, data) }
#define END_MULTI_FOR() BOOST_PP_REPEAT(NDIM, END_FOR_REP, ~)

#define FIXED_REP(z, n, d) d

#define ARR_TO_ARG_LIST_REP(z, n, arr_) arr_[n]

/*****************************************  spatial_syncer  ***************************************/
struct less_for_box : public std::binary_function<box_t, box_t, bool> {
  bool operator() (const box_t& b_0, const box_t& b_1) const
  { 
    // Note: boost::geometry::within returns true even if b_0 is on the border of b_1
    // This causes problems while using box_t as map key
    if (boost::geometry::within(b_0, b_1) ) {
      COOR_T* lcoor_0_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
      COOR_T* ucoor_0_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (b_0, min, lcoor_0_) )
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (b_0, max, ucoor_0_) )
      
      COOR_T* lcoor_1_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
      COOR_T* ucoor_1_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (b_1, min, lcoor_1_) )
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (b_1, max, ucoor_1_) )
      
      for (int i = 0; i < NDIM; i++) {
        if (lcoor_0_[i] != lcoor_1_[i] || ucoor_0_[i] != ucoor_1_[i] )
          return true;
      }
      
      // b_0 is on the border of b_1
      return false;
    }
    return false;
  }
};

struct spatial_syncer : public patch::syncer<box_t, less_for_box> {
  public:
    std::string to_str()
    {
      std::stringstream ss;
      ss << "points= \n";
      for (std::map<box_t, boost::shared_ptr<boost::condition_variable>, less_for_box >::iterator it = point_cv_map.begin(); it != point_cv_map.end(); it++)
        ss << "\t" << boost::geometry::dsv(it->first) << "\n";
    
      return ss.str();
    }
    
    int notify(box_t box)
    {
      bool contains = false;
      for (std::map<box_t, boost::shared_ptr<boost::condition_variable> >::iterator it = point_cv_map.begin(); it != point_cv_map.end(); it++) {
        // Because even when any box being waited is on the border of notified box, we confirm notification
        if (boost::geometry::within(it->first, box) ) {
          if (!contains)
            contains = true;
          
          int num_peers_to_wait = point__num_peers_map[it->first];
          --num_peers_to_wait;
          
          if (num_peers_to_wait == 0)
            point_cv_map[it->first]->notify_one();
          else
            point__num_peers_map[box] = num_peers_to_wait;
        }
      }
      
      if (contains)
        return 0;
      return 1;
    }
};

/********************************************  QTable  ********************************************/
template <typename VAL_T>
class QTable { // Query
  private:
    
  public:
    QTable() {};
    ~QTable() {};
    
    virtual std::string to_str() = 0;
    virtual int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val) = 0;
    virtual int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_) = 0;
    virtual int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v) = 0;
};

/*******************************************  RTable  *********************************************/
template <typename VAL_T>
class RTable : public QTable<VAL_T> {
  typedef std::pair<box_t, VAL_T> value;
  typedef boost::geometry::index::rtree<value, boost::geometry::index::quadratic<16> > rtree_t;
  private:
    rtree_t rtree;
  public:
    RTable() {}
    ~RTable() {}
    std::string to_str()
    {
      std::stringstream ss;
      for (typename rtree_t::const_query_iterator it = rtree.qbegin(boost::geometry::index::satisfies(boost::lambda::constant(true) ) ); it != rtree.qend(); ++it)
        ss << "\t box= " << boost::geometry::dsv(it->first) << " : " << it->second << "\n";
      
      return ss.str();
    }
    
    int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      CREATE_BOX(0, b, lcoor_, ucoor_)
      
      rtree.insert(std::make_pair(b0, val) );
      
      return 0;
    }
    
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      CREATE_BOX(0, b, lcoor_, ucoor_)
      
      std::vector<VAL_T> val_v;
      if (query(key, ver, lcoor_, ucoor_, val_v) ) {
        log_(ERROR, "query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
        return 1;
      }
      
      for (typename std::vector<VAL_T>::iterator it = val_v.begin(); it != val_v.end(); it++)
        rtree.remove(std::make_pair(b0, *it) );
      
      return 0;
    }
    
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      CREATE_BOX(0, qb, lcoor_, ucoor_)
      
      std::vector<value> value_v;
      // rtree.query(boost::geometry::index::intersects(qb0), std::back_inserter(value_v) );
      rtree.query(boost::geometry::index::contains(qb0), std::back_inserter(value_v) );
      
      for (typename std::vector<value>::iterator it = value_v.begin(); it != value_v.end(); it++)
        val_v.push_back(it->second);
      
      return (val_v.size() == 0);
    }
    
    box_t get_bounds() { return rtree.bounds(); }
    
    int get_bound_lucoor(COOR_T* &lcoor_, COOR_T* &ucoor_)
    {
      box_t box = rtree.bounds();
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (box, min, lcoor_) )
      BOOST_PP_REPEAT(NDIM, BOX_GET_CORNER_REP, (box, max, ucoor_) )
    }
};

/*******************************************  KVTable  *********************************************/
typedef std::pair<std::string, unsigned int> key_ver_pair;

template <typename VAL_T>
class KVTable : public QTable<VAL_T> { // Key Ver
  private:
    patch::thread_safe_map<key_ver_pair, std::vector<VAL_T> > kv__val_v_map;
  public:
    KVTable() {}
    ~KVTable() {}
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "kv__val_v_map= \n";
      for (typename std::map<key_ver_pair, std::vector<VAL_T> >::iterator it = kv__val_v_map.begin(); it != kv__val_v_map.end(); it++)
        ss << KV_TO_STR((it->first).first, (it->first).second) << " : " << patch::vec_to_str<>(it->second) << "\n";
      
      return ss.str();
    }
    
    int add(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, VAL_T val)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      
      key_ver_pair kv = std::make_pair(key, ver);
      if (!kv__val_v_map.contains(kv) ) {
        std::vector<VAL_T> val_v;
        kv__val_v_map[kv] = val_v;
      }
      kv__val_v_map[kv].push_back(val);
      
      return 0;
    }
    
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      
      key_ver_pair kv = std::make_pair(key, ver);
      if (!kv__val_v_map.contains(kv) )
        return 1;
      
      kv__val_v_map.del(kv);
      
      return 0;
    }
    
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<VAL_T>& val_v)
    {
      IS_VALID_BOX(lcoor_, ucoor_, return 1)
      
      key_ver_pair kv = std::make_pair(key, ver);
      if (!kv__val_v_map.contains(kv) )
        return 1;
      
      std::vector<VAL_T>& contained_val_v = kv__val_v_map[kv];
      for (typename std::vector<VAL_T>::iterator it = contained_val_v.begin(); it != contained_val_v.end(); it++)
        val_v.push_back(*it);
      
      return 0;
    }
};

/********************************************  SAlgo  *********************************************/
typedef std::pair<COOR_T*, COOR_T*> lcoor_ucoor_pair;
typedef std::pair<key_ver_pair, lcoor_ucoor_pair> kv_lucoor_pair;

typedef boost::icl::interval_set<bitmask_t> index_interval_set_t;
// typedef boost::icl::interval_map<bitmask_t, std::set<char> > index_interval__ds_id_set_map_t;

typedef char SALGO_T;
const SALGO_T SALGO_H = 'h';

class SAlgo {
  protected:
    COOR_T *lcoor_, *ucoor_;
    int hilbert_num_bits, max_index;
    
    std::vector<boost::shared_ptr<index_interval_set_t> > acced_index_interval_set_v;
    // index_interval__ds_id_set_map_t index_interval__ds_id_set_map;
  public:
    SAlgo(COOR_T* lcoor_, COOR_T* ucoor_);
    ~SAlgo() {}
    virtual std::string to_str();
    
    boost::shared_ptr<index_interval_set_t> coor_to_index_interval_set_(COOR_T* lcoor_, COOR_T* ucoor_);
    void index_interval_set_to_coor_v(const index_interval_set_t& interval_set, std::vector<COOR_T*>& coor_v);
    void expand_interval_set(bitmask_t sexpand_length, index_interval_set_t& interval_set);
    
    int add_access(COOR_T* lcoor_, COOR_T* ucoor_);
    virtual int get_to_fetch(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<lcoor_ucoor_pair>& lucoor_to_fetch_v) = 0;
};

/*****************************************  MHSAlgo  ******************************************/
// class MHSAlgo { // Markov Hilbert; tries to learn on 1d hilbert index
// };

/*****************************************  HSAlgo  *******************************************/
class HSAlgo : public SAlgo { // Hilbert
  private:
    bitmask_t sexpand_length;
    
  public:
    HSAlgo(COOR_T* lcoor_, COOR_T* ucoor_,
           bitmask_t sexpand_length);
    ~HSAlgo() {}
    std::string to_str();
    
    int get_to_fetch(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<lcoor_ucoor_pair>& lucoor_to_fetch_v);
};

#endif // _SFC_H_