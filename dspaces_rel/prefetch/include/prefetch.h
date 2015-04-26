#ifndef _PREFETCH_H_
#define _PREFETCH_H_

#include <deque>

#include "patch_pre.h"
#include "palgorithm.h"

typedef boost::function<void(std::map<std::string, std::string>)> func_handle_prefetch_cb;

typedef std::pair<std::string, unsigned int> key_ver_pair;
// pkey: prefetching key in alphabet
typedef std::pair<KEY_T, unsigned int> pkey_pver_pair;

class PBuffer { //Prefetching Buffer
  private:
    bool with_prefetch;
    size_t buffer_size;
    func_handle_prefetch_cb _handle_prefetch_cb;
    size_t app_context_size;
    
    patch_pre::thread_safe_map<int, std::deque<key_ver_pair> >  app_id__key_ver_deq_map;
    // Mostly for checking and logging
    patch_pre::thread_safe_vector<key_ver_pair> reged_key_ver_vec;
    patch_pre::thread_safe_vector<key_ver_pair> acced_key_ver_vec;
    patch_pre::thread_safe_map<int, std::vector<key_ver_pair> > app_id__acced_key_ver_vec_map;
    
    Cache<key_ver_pair> cache;
    
    // LZAlgo palgo;
    PPMAlgo ppm_algo_to_pick_app;
  public:
    PBuffer(bool with_prefetch, size_t buffer_size, func_handle_prefetch_cb _handle_prefetch_cb, 
            size_t app_context_size);
    ~PBuffer();
    std::string to_str();
    
    int reg_key_ver(std::string key, unsigned int ver, int app_id);
    int add_access(std::string key, unsigned int ver, int app_id);
    int push(std::string key, unsigned int ver);
    bool contains(std::string key, unsigned int ver);
    int get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector);
    int prefetch();
};

#endif // _PREFETCH_H_