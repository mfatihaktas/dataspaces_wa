#include "gftp_drive.h"

GFTPDriver::GFTPDriver()
{
  //
  LOG(INFO) << "GFTPDriver:: constructed.";
}

GFTPDriver::~GFTPDriver()
{
  close();
  //
  LOG(INFO) << "GFTPDriver:: destructed.";
}

void GFTPDriver::close()
{
  for (std::map<int, boost::shared_ptr<FILE> >::const_iterator it=port_serverfp_map.begin(); 
       it!=port_serverfp_map.end(); ++it) {
    int status = pclose( &(*it->second) );
    if (status == -1) {
      LOG(ERROR) << "close:: pclose errno= " << strerror(errno);
    }
  }
  //
  LOG(INFO) << "close:: done.";
}

int GFTPDriver::init_server(int port)
{
  std::string cmd = "nohup globus-gridftp-server -aa -password-file pwfile -c None ";
  cmd += "-d error,warn,info,dump,all ";
  cmd += "-port " + boost::lexical_cast<std::string>(port);
  cmd += " &";
  
  //std::cout << "cmd=\n" << cmd << std::endl;
  FILE* fp = popen(cmd.c_str(), "r");
  if (fp == NULL) {
    LOG(ERROR) << "init_server:: ERROR= " << strerror(errno);
    return 1;
  }
  
  boost::shared_ptr<FILE> fp_t(fp);
  port_serverfp_map[port] = fp_t;
  
  std::string sname = "s:" + boost::lexical_cast<std::string>(port);
  boost::shared_ptr<boost::thread> t_(
    new boost::thread(&GFTPDriver::read_print_stream, this, sname, fp)
  );
  read_print_stream_thread_v.push_back(t_);
  
  return 0;
}

void GFTPDriver::read_print_stream(std::string name, FILE* fp)
{
  int max_line_length = 100;
  char line[max_line_length];
  while (fgets(line, max_line_length, fp) != NULL)
    LOG(WARNING) << name << " >>> " << line << std::endl;
}

int GFTPDriver::put_file(std::string src_url, std::string dst_url)
{
  return gridftp_put_file(src_url.c_str(), dst_url.c_str() );
}

int GFTPDriver::get_file(std::string src_url, std::string dst_url)
{
  return gridftp_get_file(src_url.c_str(), dst_url.c_str() );
}
