#include "packet.h"
#include <glog/logging.h>

#define SIZE_SIZE 4*sizeof(char)
#define TYPE_SIZE sizeof(char)
#define TAIL_SIZE sizeof(char)
#define MAX_MSG_SIZE 512*sizeof(char)

Packet::Packet(char type, std::map<std::string, std::string> msg_map)
{
  this->type = type;
  this->msg_map = msg_map;
  //
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << msg_map;
  std::string msg_str(ss.str());
  
  this->msg_size = msg_str.length();
  this->packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
  this->data = new char[packet_size];
  
  char* temp = cast_to_chararr(SIZE_SIZE, packet_size - SIZE_SIZE );
  std::memcpy(data, temp, SIZE_SIZE);
  delete temp;
  std::memcpy(data + SIZE_SIZE, &type, TYPE_SIZE);
  std::memcpy(data + SIZE_SIZE + TYPE_SIZE, msg_str.c_str(), msg_size);
  data[packet_size - TAIL_SIZE] = '\0';
  //
  this->msg = data + TYPE_SIZE;
}

Packet::Packet(size_t type__srlzedmsgmap_size, char* type__srlzedmsgmap)
{
  this->msg_size = type__srlzedmsgmap_size - TYPE_SIZE;
  this->packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
  this->type = type__srlzedmsgmap[TYPE_SIZE - 1];
  //form the packet
  this->data = new char[packet_size];
  
  char* temp = cast_to_chararr(SIZE_SIZE, packet_size - SIZE_SIZE );
  std::memcpy(data, temp, SIZE_SIZE);
  delete temp;
  std::memcpy(data + SIZE_SIZE, type__srlzedmsgmap, TYPE_SIZE + msg_size);
  data[packet_size - TAIL_SIZE] = '\0';
  //
  this->msg = data + SIZE_SIZE + TYPE_SIZE;
  //
  std::stringstream ss;
  ss << msg;
  boost::archive::text_iarchive ia(ss);
  ia >> this->msg_map;
}

Packet::Packet(char type, char* msg)
{
  this->type = type;
  //
  this->msg_size = strlen(msg);
  if (msg_size > MAX_MSG_SIZE){
    LOG(WARNING) << "msg_size=" << msg_size << " > MAX_MSG_SIZE=" << MAX_MSG_SIZE;
    msg_size = MAX_MSG_SIZE;
  }
  packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
  //form the packet
  this->data = new char[packet_size];
  
  char* temp = cast_to_chararr(SIZE_SIZE, packet_size - SIZE_SIZE );
  std::memcpy(data, temp, SIZE_SIZE);
  delete temp;
  std::memcpy(data + SIZE_SIZE, &type, TYPE_SIZE);
  std::memcpy(data + SIZE_SIZE + TYPE_SIZE, msg, msg_size);
  data[packet_size - TAIL_SIZE] = '\0';
  //
  this->msg = data + TYPE_SIZE;
}

char* Packet::cast_to_chararr(size_t chararr_size, int number) const
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

int Packet::get_msg_size() const
{
  return msg_size;
}

int Packet::get_packet_size() const
{
  return packet_size;
}

char Packet::get_type() const
{
  return type;
}

char* Packet::get_msg() const
{
  return msg;
}

char* Packet::get_data() const
{
  return data;
}

std::map<std::string, std::string> Packet::get_msg_map() const
{
  return msg_map;
}

std::string Packet::to_str() const
{
  std::stringstream ss;
  ss << "type=" << type << "\n";
  ss << "msg=\n";
  
  for (std::map<std::string, std::string>::const_iterator it=msg_map.begin(); it!=msg_map.end(); ++it){
    ss << "\t" << it->first << ":" << it->second << "\n";
  }
  
  return ss.str();
}
