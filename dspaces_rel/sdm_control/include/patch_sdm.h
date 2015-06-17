#ifndef _PATCH_SDM_H_
#define _PATCH_SDM_H_

namespace patch_sdm {
  template <typename Tk, typename Tv>
  struct thread_safe_map
  {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv> map;
    public:
      thread_safe_map() {};
      ~thread_safe_map() {};
      
      Tv& operator[](Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map[k];
      };
      
      int del(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map.erase(map.find(k) );
        return 0;
      };
      
      bool contains(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return !(map.count(k) == 0);
      };
      
      typename std::map<Tk, Tv>::iterator begin()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.begin();
      };
      
      typename std::map<Tk, Tv>::iterator end()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.end();
      };
      
      int size() { return map.size(); }
  };
  
  template<typename Tk, typename Tv>
  std::string map_to_str(std::map<Tk, Tv> map)
  {
    std::stringstream ss;
    for (typename std::map<Tk, Tv>::const_iterator it = map.begin(); it != map.end(); it++)
      ss << "\t" << it->first << ": " << it->second << "\n";
    
    return ss.str();
  }
  
  // char* str_to_char_(std::string str)
  // {
  //   char* char_ = (char*)malloc(str.size()*sizeof(char) );
  //   strcpy(char_, str.c_str() );
    
  //   return char_;
  // }
}

#endif //_PATCH_SDM_H_