#ifndef _PATCH_DS_H_
#define _PATCH_DS_H_

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <unistd.h>

#include <string>
#include <cstdarg> // For variable argument lists
#include <csignal> // For wait signal

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
// For boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>

namespace patch_ds {
  template <typename T>
  void free_all(int num, ...)
  {
    va_list arguments;                     // A place to store the list of arguments
  
    va_start(arguments, num);           // Initializing arguments to store all values after num
    
    for (int x = 0; x < num; x++)        // Loop until all numbers are added
      va_arg(arguments, T*);
    
    va_end(arguments);                  // Cleans up the list
  }
  
  // void debug_print(std::string key, unsigned int ver, int size, int ndim,
  //                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data, size_t data_length)
  // {
  //   std::cout << "debug_print::";
  //   std::cout << "key= " << key << "\n"
  //             << "ver= " << ver << "\n"
  //             << "size= " << size << "\n"
  //             << "ndim= " << ndim << "\n";
  //   std::cout << "gdim= ";
  //   for (int i = 0; i < ndim; i++)
  //     std::cout << "\t" << gdim[i] << ", ";
  //   std::cout << "\n";
    
  //   std::cout << "lb= ";
  //   for (int i=0; i<ndim; i++)
  //     std::cout << "\t" << lb[i] << ", ";
  //   std::cout << "\n";
    
  //   std::cout << "ub= ";
  //   for (int i = 0; i < ndim; i++)
  //     std::cout << "\t" << ub[i] << ", ";
  //   std::cout << "\n";
    
  //   // 
  //   if (data == NULL)
  //     return;
    
  //   std::cout << "data_length= " << data_length << "\n";
  //   std::cout << "data= ";
  //   for (int i = 0; i < data_length; i++)
  //     std::cout << "\t" << data[i] << ", ";
  //   std::cout << "\n";
  // }
}

#endif //end of _PATCH_DS_H_
