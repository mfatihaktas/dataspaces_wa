#include "packet.h"

#include <glog/logging.h>

Packet::Packet(char type, std::map<std::string, std::string> msg_map)
{
  this->type = type;
  this->msg_map = msg_map;
  // 
  std::string msg_str;
  if (encode(msg_map, msg_str) ) {
    LOG(ERROR) << "Packet:: encode failed; msg_map= \n";
    for (std::map<std::string, std::string>::iterator it = msg_map.begin(); it != msg_map.end(); it++)
      std::cout << "\t" << it->first << " : " << it->second << "\n";
  }
  else {
    this->msg_size = msg_str.length();
    if (msg_size > MAX_MSG_SIZE) {
      LOG(WARNING) << "Packet:: msg_size=" << msg_size << " > MAX_MSG_SIZE=" << MAX_MSG_SIZE;
      msg_size = MAX_MSG_SIZE;
    }
    this->packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
    this->data_ = (char*)malloc(packet_size*sizeof(char) );
    
    char* temp = int_to_char_(SIZE_SIZE, packet_size - SIZE_SIZE);
    std::memcpy(data_, temp, SIZE_SIZE);
    free(temp);
    std::memcpy(data_ + SIZE_SIZE, &type, TYPE_SIZE);
    std::memcpy(data_ + SIZE_SIZE + TYPE_SIZE, msg_str.c_str(), msg_size);
    data_[packet_size - TAIL_SIZE] = '\0';
    // 
    this->msg_ = data_ + SIZE_SIZE + TYPE_SIZE;
  }
}

Packet::Packet(int type__srlzed_msg_map_size, char* type__srlzed_msg_map)
{
  this->msg_size = type__srlzed_msg_map_size - TYPE_SIZE;
  this->packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
  this->type = type__srlzed_msg_map[TYPE_SIZE - 1];
  // Form the packet
  this->data_ = (char*)malloc(packet_size*sizeof(char) );
  
  char* temp = int_to_char_(SIZE_SIZE, packet_size - SIZE_SIZE);
  std::memcpy(data_, temp, SIZE_SIZE);
  free(temp);
  std::memcpy(data_ + SIZE_SIZE, type__srlzed_msg_map, TYPE_SIZE + msg_size);
  data_[packet_size - TAIL_SIZE] = '\0';
  // 
  this->msg_ = data_ + SIZE_SIZE + TYPE_SIZE;
  // 
  if (decode(msg_, msg_map) )
    LOG(ERROR) << "Packet:: decode failed; msg_= " << msg_;
}

Packet::Packet(char type, char* msg_)
{
  this->type = type;
  // 
  this->msg_size = strlen(msg_);
  if (msg_size > MAX_MSG_SIZE) {
    LOG(WARNING) << "msg_size=" << msg_size << " > MAX_MSG_SIZE=" << MAX_MSG_SIZE;
    msg_size = MAX_MSG_SIZE;
  }
  packet_size = SIZE_SIZE + TYPE_SIZE + msg_size + TAIL_SIZE;
  // Form the packet
  this->data_ = (char*)malloc(packet_size*sizeof(char) );
  
  char* temp = int_to_char_(SIZE_SIZE, packet_size - SIZE_SIZE );
  std::memcpy(data_, temp, SIZE_SIZE);
  free(temp);
  std::memcpy(data_ + SIZE_SIZE, &type, TYPE_SIZE);
  std::memcpy(data_ + SIZE_SIZE + TYPE_SIZE, msg_, msg_size);
  data_[packet_size - TAIL_SIZE] = '\0';
  // 
  this->msg_ = data_ + SIZE_SIZE + TYPE_SIZE;
}

Packet::~Packet()
{
  free(data_);
  data_ = NULL;
  msg_ = NULL;
}

std::string Packet::to_str() const
{
  std::stringstream ss;
  ss << "type= " << type << "\n";
  ss << "msg= \n";
  for (std::map<std::string, std::string>::const_iterator it = msg_map.begin(); it != msg_map.end(); ++it)
    ss << "\t" << it->first << " : " << it->second << "\n";
  
  return ss.str();
}

int Packet::size() const { return packet_size; }
int Packet::get_msg_size() const { return msg_size; }
char Packet::get_type() const { return type; }
char* Packet::get_msg_() const { return msg_; }
char* Packet::get_data_() const { return data_; }
std::map<std::string, std::string> Packet::get_msg_map() const { return msg_map; }

char* Packet::int_to_char_(int char_size, int number) const
{
  std::string str = boost::lexical_cast<std::string>(number);
  int padding_size = char_size - str.length();
  if (padding_size < 0) {
    LOG(ERROR) << "int_to_char_:: padding_size= < 0";
    return NULL;
  }
  std::string final_str = std::string(padding_size, '0').append(str);
  char* arr_ = (char*)malloc(char_size*sizeof(char) );
  strcpy(arr_, final_str.c_str() );
  
  return arr_;
}

// msg: serialized std::map<std::string, std::string>
int Packet::decode(char* msg, std::map<std::string, std::string>& msg_map)
{
  try {
    std::stringstream ss;
    ss << msg;
    boost::archive::text_iarchive ia(ss);
    ia >> msg_map;
  }
  catch(boost::archive::archive_exception& e) {
    LOG(ERROR) << "decode:: exceptioon e= " << e.what();
    return 1;
  }
  
  return 0;
}

int Packet::encode(std::map<std::string, std::string> msg_map, std::string& str)
{
  try {
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << msg_map;
    
    str = ss.str();
  }
  catch(boost::archive::archive_exception& e) {
    LOG(ERROR) << "encode:: exceptioon e= " << e.what();
    return 1;
  }
  
  return 0;
}
