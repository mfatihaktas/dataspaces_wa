#ifndef _DEBUG_
#define _DEBUG_

#include <glog/logging.h>

#ifndef _STR_MACROS_
#define _STR_MACROS_
#define str_str_equals(x,y) (strcmp(x.c_str(), y.c_str() ) == 0)
#define str_cstr_equals(x, y) (strcmp(x.c_str(), (const char*)y) == 0)
#define cstr_cstr_equals(x, y) (strcmp((const char*)x, (const char*)y) == 0)

#define PAIR_TO_STR(p) "<" << (p).first << ", " << (p).second << ">"
#endif // _STR_MACROS_

#define DEBUG_IB
#ifdef DEBUG_IB
  #define log_(type, msg) \
    LOG(type) << __func__ << ":: " << msg;
    // std::cerr << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
    // std::clog << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
  #define log_s(stream, type, msg) \
    stream << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << "\n";
#else
  #define log_(type, msg) ;
  #define log_s(stream, type, msg) ;
#endif // DEBUG_IB

#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) if (x) {log_(ERROR, #x << "failed!") exit(EXIT_FAILURE); }
#define TEST_Z(x)  if (!(x)) {log_(ERROR, #x << "failed!") exit(EXIT_FAILURE); }

#define return_if_err(x, err) \
  err = x; \
  if (err) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }

#define return_err_if_ret_cond_flag(x, ret, cond, flag, err) \
  ret = x; \
  if (ret cond flag) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }
#endif // _TEST_MACROS_

#endif // _DEBUG_
