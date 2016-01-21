#include "patch_tcp.h"

namespace patch_tcp {
  // unsigned int hash_str(std::string str)
  // {
  //   unsigned int h = 31; // Also prime
  //   const char* s_ = str.c_str();
  //   while (*s_) {
  //     h = (h * HASH_PRIME) ^ (s_[0] * HASH_PRIME_2);
  //     s_++;
  //   }
  //   return h; // return h % HASH_PRIME_3;
  // }
};