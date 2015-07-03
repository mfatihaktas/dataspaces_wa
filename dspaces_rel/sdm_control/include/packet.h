#ifndef _PACKET_H_
#define _PACKET_H_

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

typedef char PACKET_T;
enum type_t {
  SDM_JOIN_REQUEST = 'J',
  SDM_JOIN_REPLY = 'j',
  SDM_JOIN_ACK = 'a',
  SDM_JOIN_NACK = 'n',
  SDM_PING = 'P',
  SDM_PONG = 'p',
  SDM_RIMSG = 'I',
  SDM_CMSG = 'C',
};

#define SIZE_SIZE 4*sizeof(char)
#define TYPE_SIZE sizeof(char)
#define TAIL_SIZE sizeof(char)
#define MAX_MSG_SIZE 512*sizeof(char)

class Packet {
  private:
    char type;
    char* msg_; //message
    std::map<std::string, std::string> msg_map;
    char* data_; //size_size + type_size + message + packet_tail
    
    int packet_size;
    int msg_size;
  public:
    Packet(char type, char* msg_);
    Packet(char type, std::map<std::string, std::string> msg_map);
    Packet(int type__srlzed_msg_map_size, char* type__srlzed_msg_map);
    ~Packet();
    std::string to_str() const;
    
    int size() const;
    int get_msg_size() const;
    char get_type() const;
    char* get_msg_() const;
    char* get_data_() const;
    std::map<std::string, std::string> get_msg_map() const;
    
    char* int_to_char_(int char_size, int number) const;
};

#endif // _PACKET_H_