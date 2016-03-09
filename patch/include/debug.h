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

#define return_if_err(x, err, a...) \
  err = x; \
  if (err) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    a \
    return err; \
  }

#define try_n_times__return_if_err(x, err, n, a...) \
for (int c = 0; c < n; c++) { \
  err = x; \
  if (err) { \
    if (c < n - 1) { \
      log_(WARNING, #x "failed, sleeping 1 sec before trying again") \
      sleep(1); \
    } \
    else { \
      log_(ERROR, #x "failed " << n << " times, no more trying!") \
      a \
      return err; \
    } \
  } \
  else \
    break; \
}

#define return_err_if_ret_cond_flag(x, ret, cond, flag, err, a...) \
  ret = x; \
  if (ret cond flag) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    a \
    return err; \
  }

#endif // _TEST_MACROS_

#endif // _DEBUG_
