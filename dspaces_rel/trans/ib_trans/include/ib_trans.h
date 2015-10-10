#ifndef _IB_TRANS_H_
#define _IB_TRANS_H_

#include "ib_server.h"
#include "ib_client.h"

#include <deque>

#define str_equals(x,y) (strcmp(x.c_str(), (const char*)y) == 0)

namespace patch_ib {
  template <typename T>
  class BQueue { //Blocking Queue
    private:
      boost::mutex mutex, mutex_pop;
      boost::condition_variable condition_pop;
      std::deque<T> d_queue;
    public:
      void push(T const& value) {
        // usleep(3*1000*1000);
        {
          boost::lock_guard<boost::mutex> guard(mutex);
          d_queue.push_front(value);
        }
        condition_pop.notify_one();
      };
      
      T pop()
      {
        while(d_queue.empty() ) {
          boost::mutex::scoped_lock lock(mutex_pop);
          condition_pop.wait(lock);
        }
        
        T rc;
        {
          boost::lock_guard<boost::mutex> guard(mutex);
          rc = d_queue.back();
          d_queue.pop_back();
        }
        return rc;
      };
      
      std::string to_str()
      {
        std::stringstream ss;
        ss << "\t ->";
        for (typename std::deque<T>::iterator it = d_queue.begin(); it != d_queue.end(); ++it)
          ss << boost::lexical_cast<std::string>(*it) << ", ";
        ss << "-> \n";
        
        return ss.str();
      };
      
      void create_timed_push_thread(T value)
      {
        boost::thread(&BQueue<T>::push, this, value);
      };
  };
}

typedef std::string RECV_ID_T;
typedef boost::function<void(RECV_ID_T, int, void*)> data_recv_cb_func;

class IBTManager { // Trans
  private:
    patch_ib::BQueue<std::string> ib_lport_queue;

  public:
    IBTManager(std::list<std::string> ib_lport_list);
    ~IBTManager();
    std::string to_str();
    
    std::string get_next_avail_ib_lport();
    void give_ib_lport_back(std::string ib_lport);
    
    void init_ib_server(std::string data_type, const char* lport,
                        RECV_ID_T recv_id, boost::function<void(RECV_ID_T, int, void*)> data_recv_cb);
    
    void init_ib_client(const char* s_laddr, const char* s_lport,
                        std::string data_type, int data_length, void* data_);
};

#endif // _IB_TRANS_H_