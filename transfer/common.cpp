/*
#----------------------------------------------------------------------------------#
#                              GRIPSI                                              #
#                             common_functions.C                                   #
#----------------------------------------------------------------------------------#
# Description : Contains the common functions used in the programs.                #
#                                                                                  #
#----------------------------------------------------------------------------------#
*/

using namespace std;
class InputConfig
{

   int i,num_records;
   int j  ,k ;

   public:
         std::string * str ,* host_string ,* host_wd_string;
         std::string * host_strings, * host_wd_strings ,* host_string_only;
         int host_count,host_wd_count;
         int readConfigFile(char *configfile);
         int validateConfigFile(char * confiffile);
         InputConfig();
         ~InputConfig();
};

InputConfig::InputConfig() {

    str=NULL;
    host_string=NULL;
    host_wd_string=NULL;
    host_strings=NULL;
    host_wd_strings=NULL;
    host_string_only=NULL;
}
/*-----------------------------------------------------------------------------------
Class         : InputConfig

Function      : ~InputConfig (destructor of the class InputConfig )

Description   : Free the memory allocated to the arrays.
-----------------------------------------------------------------------------------*/


InputConfig::~InputConfig() {

        if(str != NULL)
                delete[] str;

        if(host_string != NULL)
                delete[] host_string;

        if(host_wd_string != NULL)
                delete[] host_wd_string;
        if(host_strings != NULL)
                delete[] host_strings;

        if(host_wd_strings != NULL)
                delete[] host_wd_strings;

        if(host_string_only != NULL)
                delete[] host_string_only;
}

/*-----------------------------------------------------------------------------------
  Class       : InputConfig

  Function    : readConfigFile(...)

  Description : Reads the input from the configuration file and stores
                to the array of strings.

  Input       : configuration file as arguments.

  output      : host_string ( array of contact strings of hosts )
                host_wd_string ( array of wkdir of hosts )
                host_string_only ( array of only hostnames )
------------------------------------------------------------------------------------*/


int InputConfig::readConfigFile(char * configfile)
 {

     ifstream input_conf,fp;
     std::string buffer;
     fp.open(configfile , ios::in); // Input -- > Configuration file .
     num_records = 0;

     if(!fp) {
         cout<< "\n\t ERROR : Cannot open Configuration file : "<<endl;
         exit(-1);
     }

     while(fp) {
            getline(fp,buffer);
            if(buffer[0]!='#')
               num_records ++;
     }
     fp.close();

     str = new std::string[num_records];
     input_conf.open(configfile, ios::in); // Input -- > Configuration file .
     if(!input_conf) {
          cout<< "\n\t ERROR : Cannot open Configuration file : "<<configfile<<endl;
          exit(-1);
          //return 1;
     }
     i = 0;

     while(input_conf) {
          getline(input_conf,buffer);
          if(buffer[0] != '#') {
              str[i] = buffer;
              i++;
          }
     }
     input_conf.close();

     host_count = 0,host_wd_count =0;
     for(i = 0; i < num_records; i++){
          if(str[i].compare(0,5,"host_",0,5) == 0)
             host_count++;
          if(str[i].compare(0,11,"wkdir_host_",0,11) == 0)
             host_wd_count++;
      }

      host_strings = new std::string[host_count];
      host_wd_strings = new std::string[host_wd_count];
      j = 0,k = 0;
      for(i=0;i<num_records;i++){
        if(str[i].compare(0,5,"host_",0,5) == 0){
                host_strings[j] = str[i];
                j++;
        }
        if(str[i].compare(0,11,"wkdir_host_",0,11) == 0){
                host_wd_strings[k] = str[i];
                k++;
        }

      }

        host_string = new std::string[host_count];
        for(j=0;j<host_count;j++){
          string::size_type hostpos = host_strings[j].find("\t",0) ;
          if(hostpos != string::npos)
                  host_string[j] = host_strings[j].substr(host_strings[j].find("\t",0)+1);
          else
          {
                  cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
                  cout<<"\n\t\t           Wrong format specified for FQDN of Host "<<j+1<<endl;
                  exit(-1);
                //return 1;
          }
          hostpos = string::npos;
          host_string[j] = host_string[j].substr(0,host_string[j].find(" ",0));
     }
  host_wd_string = new std::string[host_wd_count];
     for(j=0;j<host_wd_count;j++){

          string::size_type wdpos= host_wd_strings[j].find('\t',0) ;
          if(wdpos != string::npos)
                  host_wd_string[j] = host_wd_strings[j].substr(host_wd_strings[j].find('\t')+1);
          else
          {
                  cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
                  cout<<"\n\t\t           Wrong format specified for Working Directory of Host "<<j+1<<endl;
                  exit(-1);
                  //return 1;
          }
            wdpos = string::npos;
          host_wd_string[j] = host_wd_string[j].substr(0,host_wd_string[j].find(" ",0));

          size_t slashPos,wkdirLength;
          std::string slash =("/");
          slashPos = host_wd_string[j].find_last_of(slash,host_wd_string[j].length());
          wkdirLength = host_wd_string[j].length();
          if(slashPos != wkdirLength-1)
                host_wd_string[j] = host_wd_string[j].append(slash);
   }

   host_string_only = new std::string[host_count];
   for(i=0;i<host_count;i++)
     host_string_only[i] = host_string[i].substr(0,host_string[i].find(":/",0));


    return 0;
}

/*-----------------------------------------------------------------------------------
  Class       : InputConfig

  Function    : validateConfigFile(...)

  Description : Validates the entries of the configuration file

  Input       : configuration file as arguments.

------------------------------------------------------------------------------------*/

int InputConfig::validateConfigFile(char * configfile) {

    int i,j,k,p,q;

    for(i = 0; i < host_count ; i++) {
        if(host_string[i].size() == 0) {
           cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
           cout<<"\n\t\t         Invalid Entry for FQDN of Host : "<<i+1<<endl;
           exit(-1);
           //return 1;
        }
    }

    for(i = 0; i < host_wd_count ; i++) {
        if(host_wd_string[i].size() == 0)  {
           cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
           cout<<"\n\t\t         Invalid Entry for Wkdir of Host : "<<i+1<<endl;
           exit(-1);//return 1;
        }
    }

   if(host_wd_count != host_count ) {
   cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
      cout<<"\n\t\t          No synchronization of hosts and hosts Wkdir's "<<endl;
       exit(-1);
       //return 1;
   }

   for(i = 0;i < host_count ; i++){
      for( j = 0;j < host_count ; j++){
        if(i != j){
         if(host_string[i].compare(host_string[j]) == 0) {
           p = i + 1;
           q = j + 1;
          cout<<"\n\t\t ERROR : Wrong Input given in the configuration file";
          cout<<"\n\t\t Duplicate Entries of sites :: site_0"<<p<<" & site_0"<<q<<" in ./config/grid-site-info.conf\n\n";
          exit(-1);}
       }
    }}


  return 0;
}