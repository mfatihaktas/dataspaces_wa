#ifndef PACKET_H
#define PACKET_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <map>
//for boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

class Packet{
  private:
    int packet_size;
    int msg_size;
    
    char type;
    char* msg; //message
    char* data; //size_size + type_size + message + packet_tail
    //
    std::map<std::string, std::string> msg_map;
  public:
    Packet(char type, char* msg); //DEPRECATED
    Packet(char type, std::map<std::string, std::string> msg_map);
    Packet(size_t type__srlzedmsgmap_size, char* type__srlzedmsgmap);
    ~Packet();
    
    int get_msg_size();
    int get_packet_size();
    
    char get_type();
    char* get_msg();
    char* get_data();
    std::map<std::string, std::string> get_msg_map();
    
    char* cast_to_chararr(size_t chararr_size, int number);
    std::string to_str();
};

#endif // PACKET_H