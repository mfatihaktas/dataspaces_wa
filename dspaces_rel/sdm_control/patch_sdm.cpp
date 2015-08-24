#include "patch_sdm.h"

namespace patch_sdm {
  unsigned int hash_str(std::string str)
  {
    unsigned int h = 31; // Also prime
    const char* s_ = str.c_str();
    while (*s_) {
      h = (h * HASH_PRIME) ^ (s_[0] * HASH_PRIME_2);
      s_++;
    }
    return h; // return h % HASH_PRIME_3;
  }
}
