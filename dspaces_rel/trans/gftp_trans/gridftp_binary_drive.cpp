#include "gridftp_binary_drive.h"

void handle_signal(int signum)
{
  LOG(INFO) << "handle_signal:: recved signum=" << signum;
  GridFTP::cv.notify_one();
}

boost::condition_variable GridFTP::cv;

GridFTP::GridFTP()
{
  signal(SIGINT, handle_signal);
  
  boost::shared_ptr< boost::thread > t_(
    new boost::thread(&GridFTP::hold, this)
  );
  hold_t_ = t_;
  //
  LOG(INFO) << "GridFTP:: constructed.";
}

GridFTP::~GridFTP()
{
  hold_t_->join();
  
  for (int i = 0; i < read_print_stream_thread_v.size(); i++){
    read_print_stream_thread_v[i]->join();
  }
  //
  LOG(INFO) << "GridFTP destructed.";
}

void GridFTP::hold()
{
  wait_for_flag();
  close();
}

void GridFTP::wait_for_flag()
{
  LOG(INFO) << "wait_for_flag:: waiting...";
  boost::mutex::scoped_lock lock(m);
  cv.wait(lock);

  //
  LOG(INFO) << "wait_for_flag:: done.";
}

void GridFTP::close()
{
  for (port_sstream_map::const_iterator it=p_ss_map.begin(); it!=p_ss_map.end(); ++it){
    int status = pclose(it->second);
    if (status == -1){
      LOG(ERROR) << "close:: ERROR= " << strerror(errno);
    }
  }
  
  //
  LOG(INFO) << "close:: done.";
}

int GridFTP::init_server(int port)
{
  
  std::string cmd = "nohup globus-gridftp-server -aa -password-file pwfile -c None ";
  cmd += "-d error,warn,info,dump,all ";
  cmd += "-port " + boost::lexical_cast<std::string>(port);
  cmd += " &";
  
  //std::cout << "cmd=\n" << cmd << std::endl;
  FILE* fp = popen(cmd.c_str(), "r");
  if (fp == NULL)
  {
    LOG(ERROR) << "init_server:: ERROR= " << strerror(errno);
    return 1;
  }
  p_ss_map[port] = fp;
  
  std::string sname = "s:" + boost::lexical_cast<std::string>(port);
  boost::shared_ptr< boost::thread > t_(
    new boost::thread(&GridFTP::read_print_stream, this, sname, fp)
  );
  read_print_stream_thread_v.push_back(t_);
  
  return 0;
}

int GridFTP::init_file_transfer(std::string src_url, std::string dst_url, int p, int cc)
{
  std::string p_str = boost::lexical_cast<std::string>(p);
  std::string cc_str = boost::lexical_cast<std::string>(cc);
  
  std::string cmd = "nohup globus-url-copy -vb -p " + p_str + " -cc " + cc_str;
  cmd += " " + src_url + " " + dst_url;
  cmd += " &";
  
  FILE* fp = popen(cmd.c_str(), "r");
  if (fp == NULL)
  {
    LOG(ERROR) << "init_file_transfer:: ERROR= " << strerror(errno);
    return 1;
  }
  p_cs_map[src_url] = fp;
  
  std::string sname = "c:" + src_url;
  boost::shared_ptr< boost::thread > t_(
    new boost::thread(&GridFTP::read_print_stream, this, sname, fp)
  );
  read_print_stream_thread_v.push_back(t_);
  
  return 0;
}

void GridFTP::read_print_stream(std::string name, FILE* fp)
{
  int max_line_length = 100;
  char line[max_line_length];
  while (fgets(line, max_line_length, fp) != NULL)
    LOG(WARNING) << name << " >>> " << line << std::endl;
}