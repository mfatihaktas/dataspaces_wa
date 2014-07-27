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

enum type_t {
  JOIN_REQUEST = 'J',
  JOIN_REPLY = 'j',
  JOIN_ACK = 'a',
  PING = 'P',
  PONG = 'p',
  ROUTING_TABLE_UPDATE = 'R',
  RIMSG = 'I',
};

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
    Packet(char type, char* msg);
    Packet(char type, std::map<std::string, std::string> msg_map);
    Packet(size_t type__srlzedmsgmap_size, char* type__srlzedmsgmap);
    ~Packet();
    
    int get_msg_size() const;
    int get_packet_size() const;
    
    char get_type() const;
    char* get_msg() const;
    char* get_data() const;
    std::map<std::string, std::string> get_msg_map() const;
    
    char* cast_to_chararr(size_t chararr_size, int number) const;
    std::string to_str() const;
};

#endif // PACKET_H