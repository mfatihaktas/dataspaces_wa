#ifndef IB_DELIVERY_H
#define IB_DELIVERY_H

#include "ib_server.h"
#include "ib_client.h"

#include <deque>

#define str_equals(x,y) (strcmp(x.c_str(), (const char*)y) == 0)

template <typename T>
class BQueue //Blocking Queue
{
  public:
    void push(T const& value) {
      // usleep(3*1000*1000);
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        d_queue.push_front(value);
      }
      this->d_condition.notify_one();
    };
    
    T pop() {
      while(d_queue.empty() )
      {
        boost::mutex::scoped_lock lock(this->d_mutex);
        this->d_condition.wait(lock);
      }
      
      T rc;
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        rc = this->d_queue.back();
        this->d_queue.pop_back();
      }
      return rc;
    };
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "\t->";
      for (typename std::deque<T>::iterator it = d_queue.begin(); it != d_queue.end(); ++it){
        ss << boost::lexical_cast<std::string>(*it) << ", ";
      }
      ss << "->\n";
      
      return ss.str();
    };
    
    void create_timed_push_thread(T value)
    {
      boost::thread(&BQueue<T>::push, this, value);
    };
  private:
    boost::mutex mutex;
  
    boost::mutex d_mutex;
    boost::condition_variable d_condition;
    std::deque<T> d_queue;
};

class DDManager //Data Delivery
{
  public:
    DDManager(std::list<std::string> ib_lport_list);
    ~DDManager();
    std::string to_str();
    void init_ib_server(std::string key, unsigned int ver, std::string data_type, const char* lport, data_recv_cb dr_cb);
    void init_ib_client(const char* s_laddr, const char* s_lport,
                        std::string data_type, size_t data_length, void* data_);
    
    std::string get_next_avail_ib_lport();
    void give_ib_lport_back(std::string ib_lport);
  private:
    BQueue<std::string> ib_lport_queue;
};

#endif