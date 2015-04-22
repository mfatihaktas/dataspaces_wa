#ifndef _PROFILER_H_
#define _PROFILER_H_

#include <iostream>
#include <vector>
#include <map>
#include <time.h>
#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <glog/logging.h>

namespace patch_profiler
{
  template <typename Tk, typename Tv>
  struct thread_safe_map
  {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv> map;
      typename std::map<Tk, Tv>::iterator map_it;
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
        map_it = map.find(k);
        map.erase(map_it);
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
      
      size_t size()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.size();
      };
  };
}

struct Event 
{
  private:
    std::string name;
    timeval ref_time, start_time, end_time;
    float start_time_sec, end_time_sec, dur_sec;
    
  public:
    Event() { /* LOG(INFO) << "Event:: constructed; name= " << name; */ }
    
    Event(std::string name, timeval ref_time)
    : name(name),
      ref_time(ref_time),
      dur_sec(-1)
    {
      gettimeofday(&start_time, NULL);
      start_time_sec = ( (start_time.tv_sec - ref_time.tv_sec)*1000 + (start_time.tv_usec - ref_time.tv_usec)/1000.0 )/1000.0;
      
      LOG(INFO) << "Event:: constructed; name= " << name;
    }
    
    ~Event() { /* LOG(INFO) << "Event:: destructed; name= " << name; */ }
    
    void end() 
    { 
      gettimeofday(&end_time, NULL);
      end_time_sec = ( (end_time.tv_sec - ref_time.tv_sec)*1000 + (end_time.tv_usec - ref_time.tv_usec)/1000.0 )/1000.0;
      dur_sec = end_time_sec - start_time_sec;
    }
    
    float get_dur_sec() { return dur_sec; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "name= " << name << ", dur_sec= " << boost::lexical_cast<std::string>(dur_sec)
         << ", start_time_sec= " << boost::lexical_cast<std::string>(start_time_sec)
         << ", end_time_sec= " << boost::lexical_cast<std::string>(end_time_sec);
      
      return ss.str();
    }
};

template <typename T>
class TProfiler { // Time Profiler
  private:
    patch_profiler::thread_safe_map<T, Event> event_map;
    timeval created_time;
  public:
    TProfiler() { gettimeofday(&created_time, NULL); };
    ~TProfiler() {};
    
    std::string to_str()
    {
      std::stringstream ss;
      
      float total_dur_sec = 0;
      for (typename std::map<T, Event>::iterator it = event_map.begin(); it != event_map.end(); it++) {
        total_dur_sec += (it->second).get_dur_sec();
        ss << "\t" << (it->second).to_str() << "\n";
      }
      ss << "\t" << "total_dur_sec= " << total_dur_sec << "\n";
      
      return ss.str();
    }
    
    void add_event(T event_key, std::string event_name)
    {
      Event e(event_name, created_time);
      event_map[event_key] = e;
    }
    
    void end_event(T event_key)
    {
      event_map[event_key].end();
    }
    
    void get_event_dur_vector(float& total_dur, std::vector<T>& event_dur_v)
    {
      for (typename std::map<T, Event>::iterator it = event_map.begin(); it != event_map.end(); it++) {
        float event_dur = (it->second).get_dur_sec();
        total_dur += event_dur;
        event_dur_v.push_back(event_dur);
      }
    }
};

#endif // _PROFILER_H_