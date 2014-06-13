#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "dht_client.h"
#include "packet.h"

int main(int argc, char **argv){
  DHTClient dhtc((char*)"localhost", 6000);
  dhtc.connect();
  
  Packet packet('J', (char*)"SOS");
  char* data = packet.get_data();
  dhtc.send(packet.get_packet_size(), packet.get_data());
  
  /*
  Packet packet('J', "SOS");
  char* data = packet.get_data();
  std::cout << "data=" << data << std::endl;
  */
  //
  std::cout << "Enter\n";
  std::cin.ignore();
	return 0;
}
