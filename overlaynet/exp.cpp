#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "dht_server.h"

char* data;

void handle_read()
{
  std::cout << "handle_read; data=\n" << data << std::endl;
}

int main(int argc, char **argv){
  boost::function<void(void)> fp = boost::bind(handle_read);
  DHTServer dhts( (char*)"localhost", 6000, &data, fp );
  dhts.init_listen();
  
  delete data;
  dhts.close();
  //
  std::cout << "Enter\n";
  std::cin.ignore();
	return 0;
}
