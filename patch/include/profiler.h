#ifndef _PROFILER_H_
#define _PROFILER_H_

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <time.h>
#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <glog/logging.h>

#include "patch.h"

struct Event {
  private:
    std::string name;
    timeval ref_time, start_time, end_time;
    float start_time_sec, end_time_sec, dur_sec;
    
  public:
    Event() { /* log_(INFO, "constructed; name= " << name) */ }
    
    Event(std::string name, timeval ref_time)
    : name(name), ref_time(ref_time),
      dur_sec(-1)
    {
      gettimeofday(&start_time, NULL);
      start_time_sec = ( (start_time.tv_sec - ref_time.tv_sec)*1000 + (start_time.tv_usec - ref_time.tv_usec)/1000.0 )/1000.0;
      
      log_(INFO, "constructed; name= " << name)
    }
    
    ~Event() { /* log_(INFO, "destructed; name= " << name) */ }
    
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
      ss << "name= " << name << ", dur_sec= " << dur_sec;
        // << ", start_time_sec= " << start_time_sec
        // << ", end_time_sec= " << end_time_sec;
      
      return ss.str();
    }
};

template <typename T>
class TProfiler { // Time Profiler
  private:
    patch::thread_safe_map<T, Event> key_event_map;
    timeval created_time;
  public:
    TProfiler() { gettimeofday(&created_time, NULL); }
    ~TProfiler() {}
    
    std::string to_str() {
      std::stringstream ss;
      
      int num_dur = 0;
      float total_dur_sec = 0;
      for (typename std::map<T, Event>::iterator it = key_event_map.begin(); it != key_event_map.end(); it++) {
        ss << "\t" << (it->second).to_str() << "\n";
        float dur = (it->second).get_dur_sec();
        if (dur > 0) {
          total_dur_sec += dur;
          ++num_dur;
        }
      }
      ss << "\t total_dur_sec= " << total_dur_sec << ", avg_dur_sec= " << total_dur_sec / num_dur << "\n";
      
      return ss.str();
    }
    
    std::string to_brief_str() {
      std::stringstream ss;
      
      int num_dur = 0;
      float total_dur_sec = 0;
      int counter = 0;
      for (typename std::map<T, Event>::iterator it = key_event_map.begin(); it != key_event_map.end(); it++) {
        if (counter == 0) {
          ++counter;
          continue;
        }
        float dur = (it->second).get_dur_sec();
        if (dur > 0) {
          total_dur_sec += dur;
          ++num_dur;
        }
      }
      ss << "\t total_dur_sec= " << total_dur_sec << ", avg_dur_sec= " << total_dur_sec / num_dur << "\n";
      
      return ss.str();
    }
    
    int add_event(T event_key, std::string event_name) {
      if (key_event_map.contains(event_key) ) {
        log_(WARNING, "ALREADY in; event_key= " << event_key)
        return 1;
      }
      
      Event e(event_name, created_time);
      key_event_map[event_key] = e;
      return 0;
    }
    
    int end_event(T event_key) {
      if (!key_event_map.contains(event_key) ) {
        log_(WARNING, "NOT in; event_key= " << event_key)
        return 1;
      }
      key_event_map[event_key].end();
      return 0;
    }
    
    void get_event_dur_vector(float& total_dur, std::vector<T>& event_dur_v) {
      for (typename std::map<T, Event>::iterator it = key_event_map.begin(); it != key_event_map.end(); it++) {
        float event_dur = (it->second).get_dur_sec();
        total_dur += event_dur;
        event_dur_v.push_back(event_dur);
      }
    }
};

// Lets multiple events with the same key to be added, computes statistics with respect to each key
template <typename T>
class SProfiler { // Statistics Profiler
  typedef std::pair<T, int> key_ver_pair;
  private:
    patch::thread_safe_map<key_ver_pair, Event> kv_event_map;
    patch::thread_safe_set<T> key_s;
    timeval created_time;
  public:
    SProfiler() { gettimeofday(&created_time, NULL); }
    ~SProfiler() {}
    
    std::string to_str() {
      std::stringstream ss;
      for (typename std::map<key_ver_pair, Event>::iterator it = kv_event_map.begin(); it != kv_event_map.end(); it++)
        ss << PAIR_TO_STR(it->first) << ": " << (it->second).to_str() << "\n";
    
      return ss.str();
    }
    
    std::string to_brief_str() {
      std::stringstream ss;
      
      for (typename std::set<T>::iterator it = key_s.begin(); it != key_s.end(); it++) {
        std::vector<float> dur_v;
        int ver = 0;
        while (kv_event_map.contains(std::make_pair(*it, ver) ) ) {
          Event& e = kv_event_map[std::make_pair(*it, ver) ];
          dur_v.push_back(e.get_dur_sec() );
          ver++;
        }
        
        float total_dur = 0;
        for (std::vector<float>::iterator _it = dur_v.begin(); _it != dur_v.end(); _it++)
          total_dur += *_it;
        float mean = (float)total_dur / dur_v.size();
        
        float total_dur_sqr = 0;
        for (std::vector<float>::iterator _it = dur_v.begin(); _it != dur_v.end(); _it++)
          total_dur_sqr += pow(*_it - mean, 2);
        float stdev = (float)total_dur_sqr / dur_v.size();
        
        ss << "\t" << *it << ": mean= " << mean << ", stdev= " << stdev << "\n";
      }
      
      return ss.str();
    }
    
    int add_event(T event_key, std::string event_name) {
      int ver = 0;
      while (ver++ && kv_event_map.contains(std::make_pair(event_key, ver) ) );
      
      key_s.add(event_key);
      Event e(event_name, created_time);
      kv_event_map[std::make_pair(event_key, ver) ] = e;
      return 0;
    }
    
    int end_event(T event_key) {
      int ver = 0;
      while (ver++ && kv_event_map.contains(std::make_pair(event_key, ver) ) );
      if (ver == 0) {
        log_(WARNING, "NOT in; event_key= " << event_key)
        return 1;
      }
      
      kv_event_map[std::make_pair(event_key, --ver) ].end();
      return 0;
    }
};

#endif // _PROFILER_H_
