#ifndef MSG_HPP
#define MSG_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define header_length 4
#define max_body_length 512

class Msg{
  private:
    char data_[header_length + max_body_length];
    size_t body_length_;
  public:
    Msg() : body_length_(0)
    {
    }
    //
    size_t get_msg_length() const{
      return header_length + body_length_;
    }
    size_t get_header_length() const{
      return header_length;
    }
    size_t get_body_length() const{
      return body_length_;
    }
    void set_body_length(size_t length)
    {
      body_length_ = length;
      if (body_length_ > max_body_length)
        printf("body_length_=%s > max_body_length=%s", body_length_, max_body_length);
        body_length_ = max_body_length;
    }
    
    //
    char* get_data(){
      return data_;
    }
    char* get_body()
    {
      return data_ + header_length;
    }


    bool decode_header()
    {
      using namespace std; // For strncat and atoi.
      char header[header_length + 1] = "";
      strncat(header, data_, header_length);
      body_length_ = atoi(header);
      if (body_length_ > max_body_length)
      {
        body_length_ = 0;
        return false;
      }
      return true;
    }

    void encode_header()
    {
      using namespace std; // For sprintf and memcpy.
      char header[header_length + 1] = "";
      sprintf(header, "%4d", body_length_);
      memcpy(data_, header, header_length);
    }
};

#endif // MSG_HPP
