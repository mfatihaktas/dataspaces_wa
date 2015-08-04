#ifndef _PATCH_SFC_H_
#define _PATCH_SFC_H_

namespace patch_sfc {
  template<typename T>
  std::string vec_to_str(std::vector<T> v)
  {
    std::stringstream ss;
    for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); it++) {
      ss << boost::lexical_cast<std::string>(*it);
      if (it != (v.end() - 1) )
        ss << ", ";
    }
    
    return ss.str();
  }
  
  template<typename T>
  std::string set_to_str(std::set<T> s)
  {
    std::stringstream ss;
    for (typename std::set<T>::iterator it = s.begin(); it != s.end(); it++)
      ss << boost::lexical_cast<std::string>(*it) << ", ";
    
    return ss.str();
  };
  
  template <typename T>
  std::string arr_to_str(int size, T* arr_)
  {
    std::stringstream ss;
    
    for (int i = 0; i < size; i++) {
      ss << boost::lexical_cast<std::string>(arr_[i] );
      if (i < size - 1)
        ss << ",";
    }
    
    return ss.str();
  }
  
  template<typename Tk, typename Tv>
  std::string map_to_str(std::map<Tk, Tv> m)
  {
    std::stringstream ss;
    for (typename std::map<Tk, Tv>::iterator it = m.begin(); it != m.end(); it++)
      ss << "\t" << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
    
    return ss.str();
  }
}

/******************************************  MACROS  *********************************************/
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

#define LUCOOR_TO_STR(lcoor_, ucoor_) \
     "\t lcoor_= " << patch_sfc::arr_to_str<>(NDIM, lcoor_) \
  << ", ucoor_= " << patch_sfc::arr_to_str<>(NDIM, ucoor_)

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

#endif // _PATCH_SFC_H_