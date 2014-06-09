#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "dht_server.h"

int main(int argc, char **argv){
  DHTServer dhts((char*)"localhost", 6000);
  //DHTClient dhtc("localhost", 6000);
  dhts.init_listen();
  
	return 0;
}
