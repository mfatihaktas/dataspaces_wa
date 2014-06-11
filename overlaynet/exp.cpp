#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <boost/thread.hpp>

#include "dht_server.h"

void handle_read(char data_type, char* data)
{
  std::cout << "handle_read; data=\n" << data << std::endl;
  if (data_type == 'd'){ //dynamic
    delete data;
  }
}

int main(int argc, char **argv){
  
  boost::function<void(char, char*)> fp = boost::bind(handle_read, _1, _2);
  DHTServer dhts( (char*)"localhost", 6000, fp );
  dhts.init_listen();
  
  /*
  boost::thread worker_thread(handle_read, 's', (char*)"hello");
  worker_thread.join();
  */
  //
  std::cout << "Enter\n";
  std::cin.ignore();
	return 0;
}
