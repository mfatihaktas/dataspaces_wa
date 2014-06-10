#include "packet.h"
#include <glog/logging.h>

#define SIZE_HEADER_SIZE 4
#define TYPE_HEADER_SIZE 1
#define MAX_MSG_SIZE 512

Packet::Packet(size_t msg_size, char* msg)
{
  header = msg[TYPE_HEADER_SIZE-1];
  data = msg+TYPE_HEADER_SIZE;
  packet_size = msg_size + SIZE_HEADER_SIZE;
}

Packet::Packet(char header, const char* msg)
{
  this->header = header;
  //
  size_t msg_size = strlen(msg);
  if (msg_size > MAX_MSG_SIZE){
    LOG(WARNING) << "msg_size=" << msg_size << " > MAX_MSG_SIZE=" << MAX_MSG_SIZE;
    msg_size = MAX_MSG_SIZE;
  }
  packet_size = SIZE_HEADER_SIZE + TYPE_HEADER_SIZE + msg_size + 1;
  //form the packet
  data = new char[packet_size];
  
  char* temp = cast_to_chararr(SIZE_HEADER_SIZE, packet_size - SIZE_HEADER_SIZE );
  std::memcpy(data, temp, SIZE_HEADER_SIZE);
  delete temp;
  std::memcpy(data+SIZE_HEADER_SIZE, &header, TYPE_HEADER_SIZE);
  std::memcpy(data+SIZE_HEADER_SIZE+TYPE_HEADER_SIZE, msg, msg_size);
  data[packet_size-1] = '\0';
}

char* Packet::cast_to_chararr(size_t chararr_size, int number)
{
  std::string str = boost::lexical_cast<std::string>(number);
  size_t padding_size = chararr_size - str.length();
  if (padding_size < 0){
    LOG(ERROR) << "padding_size=" << " < 0";
    return NULL;
  }
  std::string final_str = std::string(padding_size, '0').append(str);
  char* arr = new char[chararr_size];
  strcpy(arr, final_str.c_str());
  return arr;
}

Packet::~Packet()
{
  delete data;
}

int Packet::get_packet_size()
{
  return packet_size;
}

char* Packet::get_data()
{
  return data;
}