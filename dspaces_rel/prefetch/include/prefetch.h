#ifndef _PREFETCH_H_
#define _PREFETCH_H_

#include "palgorithm.h"

typedef boost::function<void(std::map<std::string, std::string>)> func_handle_prefetch_cb;

typedef std::pair<std::string, unsigned int> key_ver_pair;
// pkey: prefetching key in alphabet
typedef std::pair<char, unsigned int> pkey_pver_pair;

class PBuffer { //Prefetching Buffer
  private:
    size_t buffer_size;
    func_handle_prefetch_cb _handle_prefetch_cb;
    char* alphabet_;
    size_t alphabet_size;
    patch::thread_safe_map<key_ver_pair, pkey_pver_pair>  key_ver__pkey_pver_map;
    patch::thread_safe_map<pkey_pver_pair, key_ver_pair>  pkey_pver__key_ver_map;
    Cache<pkey_pver_pair> cache;
    
    std::vector<key_ver_pair> accessed_key_ver_vector;
    std::vector<key_ver_pair> buffered_key_ver_vector;
    
    // LZAlgo palgo;
    PPMAlgo palgo;
    
  public:
    PBuffer(size_t buffer_size, func_handle_prefetch_cb _handle_prefetch_cb, 
            char* alphabet_, size_t alphabet_size, size_t context_size);
    ~PBuffer();
    std::string to_str();
    
    int reg_key_ver__pkey_pver_pair(std::string key, unsigned int ver);
    int add_access(std::string key, unsigned int ver);
    int push(std::string key, unsigned int ver);
    bool contains(std::string key, unsigned int ver);
    int get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector);
    int prefetch();
};

#endif // _PREFETCH_H_