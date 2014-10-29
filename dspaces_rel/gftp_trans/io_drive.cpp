#include "io_drive.h"

IODriver::IODriver(std::string working_dir)
{
  this->working_dir = working_dir;
  //
  LOG(INFO) << "IODriver:: constructed;\n" << to_str();
}

IODriver::~IODriver()
{
  //
  LOG(INFO) << "IODriver:: destructed.";
}

std::string IODriver::to_str()
{
  std::stringstream ss;

  ss << "\tworking_dir= " << working_dir << "\n";
  ss << "\n";
  
  return ss.str();
}

int IODriver::write_file(std::string another_working_dir, std::string file_name, size_t datasize_inB, void* data_)
{
  std::string working_dir_;
  if (another_working_dir.empty() ) {
    working_dir_ = this->working_dir;
  }
  else {
    working_dir_ = another_working_dir;
  }
  std::string file_dir_name = working_dir_ + file_name;
  // 
  std::ofstream fout(file_dir_name.c_str(), std::ofstream::binary);
  if (!fout.is_open() ) {
    LOG(ERROR) << "write_file:: unable to open file_dir_name= " << file_dir_name;
    return 1;
  }
  
  fout.write(reinterpret_cast<char*>(data_), datasize_inB);
  fout.close();
  // 
  LOG(INFO) << "write_file:: done file_dir_name= " << file_dir_name << ", datasize_inB= " << datasize_inB;
  return 0;
}

size_t IODriver::read_file(std::string another_working_dir, std::string file_name, void* &data_)
{
  std::string working_dir_;
  if (another_working_dir.empty() ) {
    working_dir_ = this->working_dir;
  }
  else {
    working_dir_ = another_working_dir;
  }
  std::string file_dir_name = working_dir_ + file_name;
  // 
  std::ifstream fin (file_dir_name.c_str(), std::ifstream::binary);
  
  fin.seekg (0, fin.end);
  size_t datasize_inB = fin.tellg();
  fin.seekg (0, fin.beg);
  
  char* buffer_ = new char[datasize_inB];
  fin.read(buffer_, datasize_inB);
  if (fin) {
    LOG(INFO) << "read_file:: done, file_dir_name= " << file_dir_name << ", datasize_inB= " << datasize_inB;
    fin.close();
  }
  else {
    LOG(ERROR) << "read_file:: could not read all!, datasize_inB= " << datasize_inB << ", read_datasize_inB= " << fin.gcount();
    fin.close();
    return fin.gcount();
  }
  
  data_ = buffer_;
  // 
  return datasize_inB;
}