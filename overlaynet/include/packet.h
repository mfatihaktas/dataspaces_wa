#ifndef PACKET_H
#define PACKET_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/lexical_cast.hpp>

class Packet{
  private:
    int packet_size;
    char header;
    char* data;
  public:
    Packet(char header, const char* msg);
    Packet(size_t msg_size, char* msg);
    ~Packet();
    int get_packet_size();
    char* get_data();
    char get_header();
    char* cast_to_chararr(size_t chararr_size, int number);
};

#endif // PACKET_H