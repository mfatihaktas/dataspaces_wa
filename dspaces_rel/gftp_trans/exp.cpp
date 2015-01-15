#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <glog/logging.h>

#include "gftp_delivery.h"
// #include "gftp_drive.h"
/*
#include "gridftp_binary_drive.h"
extern "C" {
  #include "gridftp_api_drive.h"
}
*/
// #include "io_drive.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"s_lintf", optional_argument, NULL, 1},
    {"s_laddr", optional_argument, NULL, 2},
    {"s_lport", optional_argument, NULL, 3},
    {"tmpfs_dir", optional_argument, NULL, 4},
    {"s_tmpfs_dir", optional_argument, NULL, 5},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s",
                     long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["s_lintf"] = optarg;
        break;
      case 2:
        opt_map["s_laddr"] = optarg;
        break;
      case 3:
        opt_map["s_lport"] = optarg;
        break;
      case 4:
        opt_map["tmpfs_dir"] = optarg;
        break;
      case 5:
        opt_map["s_tmpfs_dir"] = optarg;
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc){
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  //
  std::cout << "opt_map=\n";
  for (std::map<std::string, std::string>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int test_io_driver(IODriver& io_driver)
{
  int datasize = 100;
  int datasize_inB = datasize*sizeof(int);
  int* data_ = (int*)malloc(datasize_inB);
  for (int i = 0; i < datasize; i++) {
    data_[i] = i + 1;
  }
  
  if (io_driver.write_file("", "deneme.dat", datasize_inB, data_) ) {
    LOG(ERROR) << "main:: io_driver.write_file failed!";
  }
  
  void* _data_;
  int _datasize_inB = io_driver.read_file("", "deneme.dat", _data_);
  int* int_data_ = static_cast<int*>(_data_);
  int _datasize = _datasize_inB / sizeof(int);
  LOG(INFO) << "main:: _data=";
  for (int i = 0; i < _datasize; i++) {
    std::cout << int_data_[i] << " ";
  }
  std::cout << "\n";
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  // std::string file_dir = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/gftp_trans/dummy";
  // std::string file_dir = "/dev/shm";
  
  size_t datasize = 1*1*1000;
  
  if (opt_map["type"].compare("s") == 0) {
    GFTPDDManager gftpdd_manager(opt_map["s_lintf"], boost::lexical_cast<int>(opt_map["s_lport"]), opt_map["s_tmpfs_dir"]);
    gftpdd_manager.init_gftp_server();
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("g") == 0) {
    // gridftp_get_file(opt_map["src_url"].c_str(), opt_map["dst_url"].c_str() );

    // IODriver io_driver(file_dir + "/server");
    // void* data_;
    // size_t datasize_inB = io_driver.read_file("", "/ds_dummy_0.dat", data_);
    // LOG(INFO) << "main:: datasize_inB= " << datasize_inB;
    // int* int_data_ = static_cast<int*>(data_);
    // int datasize = datasize_inB / sizeof(int);
    // LOG(INFO) << "main:: datasize=" << datasize << ", data_=";
    // for (int i = 0; i < datasize; i++) {
    //   std::cout << int_data_[i] << " ";
    // }
    // std::cout << "\n";
    
    // gftpdd_manager.get_over_gftp("127.0.0.1", "5000", file_dir + "server",
    //                               "dummy", 0, datasize_inB, data_);
    
    size_t datasize_inB;
    void* data_;
    GFTPDDManager gftpdd_manager("", boost::lexical_cast<int>(opt_map["s_lport"]), opt_map["tmpfs_dir"]);
    gftpdd_manager.get_over_gftp(opt_map["s_laddr"], opt_map["s_lport"], opt_map["s_tmpfs_dir"],
                                "dummy", 0, datasize_inB, data_);
    int* int_data_ = static_cast<int*>(data_);
    int datasize = datasize_inB / sizeof(int);
    LOG(INFO) << "main:: datasize=" << datasize << ", data_=";
    for (int i = 0; i < datasize; i++) {
      std::cout << int_data_[i] << " ";
    }
    std::cout << "\n";
    
    free(data_);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("p") == 0) {
    // gridftp_put_file(opt_map["src_url"].c_str(), opt_map["dst_url"].c_str() );
    
    // IODriver io_driver("/cac/u01/mfa51/Desktop/dataspaces_wa/gftp_trans/dummy/");
    // test_io_driver(io_driver);
    
    size_t datasize_inB = datasize*sizeof(int);
    int* data_ = (int*)malloc(datasize_inB);
    for (int i = 0; i < datasize; i++) {
      data_[i] = i + 1;
    }
    LOG(INFO) << "main:: datasize_inB= " << datasize_inB;
    GFTPDDManager gftpdd_manager("", boost::lexical_cast<int>(opt_map["s_lport"]), opt_map["tmpfs_dir"]);
    gftpdd_manager.put_over_gftp(opt_map["s_laddr"], opt_map["s_lport"], opt_map["s_tmpfs_dir"],
                                 "dummy", 0, datasize_inB, data_);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  //gridftp_put_file( (char*)(opt_map["src_url"].c_str()), (char*)(opt_map["dst_url"].c_str()) );
  //gridftp_fancy_put_file( (char*)(opt_map["src_url"].c_str()), (char*)(opt_map["dst_url"].c_str()), 2);
  
  return 0;
}