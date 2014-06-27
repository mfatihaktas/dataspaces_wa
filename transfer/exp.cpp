#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include "ds_client.cpp"

void dummy_func()
{
  globus_result_t status = (globus_result_t)globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);

  if (status != GLOBUS_SUCCESS){
    std::string tmpstr = globus_object_printable_to_string(globus_error_get(status));
    std::cout<<"\n\t Error: Failed to load GLOBUS_FTP_CLIENT_MODULE.\n\t Error Code "<<status<<"\n\t"<<tmpstr<<std::endl;
    exit(1);
  }
  
  std::cout << "Hehe\n";
  
  globus_module_deactivate_all(); 
}

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"src_url", required_argument, NULL, 0},
    {"dst_url", required_argument, NULL, 1},
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
      case 's':
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
  google::InitGoogleLogging("exp");
  //
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  DSClient ds_client;
  ds_client.send_file(opt_map["src_url"], opt_map["dst_url"]);
  
  return 0;
}