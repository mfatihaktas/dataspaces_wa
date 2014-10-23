#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <glog/logging.h>

#include "gftp_drive.h"
/*
#include "gridftp_binary_drive.h"
extern "C" {
  #include "gridftp_api_drive.h"
}
*/
#include "io_drive.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"src_url", optional_argument, NULL, 0},
    {"dst_url", optional_argument, NULL, 1},
    {"port", optional_argument, NULL, 2},
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
        opt_map["src_url"] = optarg;
        break;
      case 1:
        opt_map["dst_url"] = optarg;
        break;
      case 2:
        opt_map["port"] = optarg;
        break;
      case 's':
        opt_map["s"] = "s";
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

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  GFTPDriver gftp_driver;
  if (opt_map.count("s") ) {
    gftp_driver.init_server(5000);
  }
  else {
    // gftp_driver.init_file_transfer(opt_map["src_url"], opt_map["dst_url"]);
    // gridftp_put_file(opt_map["src_url"].c_str(), opt_map["dst_url"].c_str() );
    IODrive io_drive("/cac/u01/mfa51/Desktop/dataspaces_wa/gftp_trans/dummy/");
    int datasize = 100;
    int datasize_inB = datasize*sizeof(int);
    int* data_ = (int*)malloc(datasize_inB);
    for (int i = 0; i < datasize; i++) {
      data_[i] = i + 1;
    }
    
    if (io_drive.write_file("", "deneme.dat", datasize_inB, data_) ) {
      LOG(ERROR) << "main:: io_drive.write_file failed!";
    }
    
    void* _data_;
    int _datasize_inB = io_drive.read_file("", "deneme.dat", _data_);
    int* int_data_ = static_cast<int*>(_data_);
    int _datasize = _datasize_inB / sizeof(int);
    LOG(INFO) << "main:: _data=";
    for (int i = 0; i < _datasize; i++) {
      std::cout << int_data_[i];
    }
    std::cout << "\n";
  }
  
  std::cout << "Enter\n";
  getline(std::cin, temp);
  /*
  */
  //gridftp_put_file( (char*)(opt_map["src_url"].c_str()), (char*)(opt_map["dst_url"].c_str()) );
  //gridftp_fancy_put_file( (char*)(opt_map["src_url"].c_str()), (char*)(opt_map["dst_url"].c_str()), 2);
  
  return 0;
}