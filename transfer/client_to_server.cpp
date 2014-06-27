/*
#---------------------------------------------------------------------------------------------#
#                            gridftpClientToServer.C                                             #
#---------------------------------------------------------------------------------------------#
#  Description : Transfer the data file from client site to each remote site.                 #
#                and return the status of the file transfer.                                  #
#                                                                                             #
#  Input       : Source URL string of the source server , and the Destination URL             #
#                string of the destination server .                                           #
#                The format for URL string is                                                 #
#                Source URL      : full path of the file to transferred                       #
#                Destination URL : gsiftp://server-hostname/fullpathname/ofthefile/tobetransfered
#
# Output      : Displays whether the transfer is SUCCESSFUL/FAILED.                           #
#                                                                                             #
#---------------------------------------------------------------------------------------------#
*/

#include <fcntl.h>
#include "iomanip"

#include "globus_ftp_client.h"
#include "globus_common.h"

#include <iostream>
#include <string>
#include <fstream>

#include "common.cpp"
#include "globus_callback.cpp"
using namespace std;
//FILE *fd;
#define MAX_BUFFER_SIZE 2048
#define ERROR -1
#define SUCCESS 0


class GridFTP:public InputConfig,public GlobusCallback {

        globus_ftp_client_handleattr_t            hattr;
        globus_ftp_client_operationattr_t         oattr;
        globus_ftp_client_handle_t                handle;
        globus_byte_t                             buffer[MAX_BUFFER_SIZE];
        globus_size_t                             buffer_length ;
        globus_result_t                           status;
        char *                                    tmpstr;

public :
        GridFTP();
        ~GridFTP();
        int gridftpClientToServer(string  ,string);

};

/*-----------------------------------------------------------------------------------------------------

Function     : done_cb(...)

Used         : gridftpClientToServer(...),gridftpThirdPartyTransfer(...),gridftpServerToClient(...)

Description  : It is a callback function ,it is called when the transfer is
               completely finished, i.e. both the data channel and control channel
               exchange.
               Here it simply sets a global variable (done) to true so the main
               program will exit the while loop.
------------------------------------------------------------------------------------------------------*/

namespace gridftp_cb {

  static void done_cb ( void *user_arg , globus_ftp_client_handle_t *handle , globus_object_t *err) {

            GridFTP *monitor=(GridFTP*)user_arg;   

          char * tmpstr;
          if ( err ){ 
               cout<<"\t\t Status : File Transferred Failed "<<endl;
                  cout<<"\t\t ERROR :"<<globus_object_printable_to_string(err)<<endl;
                 }
          else 
              cout<<"\t\t Status : File Transferred Successfully"<<endl;
                
    monitor->setDoneValue();
          return;
  };

/*--------------------------------------------------------------------------------------------------------

Function    : data_cb(...)

Used        : gridftpClientToServer(..)

Description : read or write operation in the FTP Client library is asynchronous.A
              callback of this type is passed to such data  operation function calls.
              It is called when the user supplied buffer has been successfully
              transferred to the kernel.
              Note: That does not mean it has been successfully transmitted,instead it
              just reads the next block of data and calls register_write/register_read again.
-----------------------------------------------------------------------------------------------------------*/

static void data_cb (void *user_arg , globus_ftp_client_handle_t *handle,globus_object_t * err, globus_byte_t *buffer,
                     globus_size_t length , globus_off_t offset,globus_bool_t  eof)
{
        if ( err ) {
                cout<<"\t\t ERROR :"<<globus_object_printable_to_string(err)<<endl;
        }
        else {
                if ( !eof ) {
                        FILE *fd = (FILE *) user_arg;
                        int rc;
                        rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
                        if ( ferror(fd) != SUCCESS) {
                                 cout<<"\n\t\t Error : Function data_cb\n"<<"\t\t Error code :"<< errno<<endl;
                                return;
                        }
           globus_ftp_client_register_write(handle, buffer,rc,offset + length,feof(fd) != SUCCESS,                                                          data_cb, (void *) fd);
                }
         }
         return;
 };
}
/*---------------------------------------------------------------------------------------------
Class       : GridFTP

Function    : GridFTP ( Constructor of class GridFTP )

Description : Used to
              a) Activate the GridFTP client module
              b) Initialize the mutex lock and condition variables
              c) Initialize the GridFTP client handle
-----------------------------------------------------------------------------------------------*/

GridFTP::GridFTP() {

        buffer_length = MAX_BUFFER_SIZE;
        status = (globus_result_t)globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);

        if ( status != GLOBUS_SUCCESS ) {

                tmpstr = globus_object_printable_to_string(globus_error_get(status));
                cout<<"\n\t Error: Failed to load GLOBUS_FTP_CLIENT_MODULE.\n\t Error Code "<<status<<"\n\t"<<tmpstr<<endl;
                exit(1);
        }

};
/*--------------------------------------------------------------------------------------------
Class       : GridFTP

Function    : ~GridFTP (Destructor of class GridFTP )

Description : Used to
              a) Destroy the GridFTP client handle
              b) Deactivate the GridFTP client module
----------------------------------------------------------------------------------------------*/

GridFTP::~GridFTP() {
        globus_module_deactivate_all();

};
/*----------------------------------------------------------------------------------------------
cLass        : GridFTP
Function     : gridftpClientToServer(...)
Description  : To transfer the file from client site to remote site
Input        : a) Source file path which is to transferred
               b) Destination url to store the file
               c) LogFile
-----------------------------------------------------------------------------------------------*/

int GridFTP::gridftpClientToServer(string  src,string  dst) {

        int rc;
        FILE *fd;

        /* Initialize the handle attribute */
        if (globus_ftp_client_handleattr_init(&hattr) != GLOBUS_SUCCESS) {
                cout<<"\n\t\t ERROR : Failed to activate the ftp client handleattr\n";
                return 1;
        }

        /* Initialize the operation attribute */
        if (globus_ftp_client_operationattr_init(&oattr) != GLOBUS_SUCCESS) {
                cout<<"\n\t\t ERROR : Failed to initialize operationattr\n";
                return 1;
        }

        /* Initalize the handle */
   if (globus_ftp_client_handle_init(&handle,&hattr) != GLOBUS_SUCCESS) {
                cout<<"\n\t\t ERROR : Failed to initialize the handle\n";
                return 1;
        }

        continueOnCond();

          fd = fopen(src.c_str(),"r");
          if ( fd == NULL ) {
              cout<<"Error in opening local file"<<src;
             return 1;
          }

        /* Gridftp API call to start the put operation */
        status = globus_ftp_client_put(&handle,dst.c_str(),GLOBUS_NULL,GLOBUS_NULL,gridftp_cb::done_cb,this);
        if ( status != GLOBUS_SUCCESS ) {

           globus_object_t * err;
           err = globus_error_get(status);
           cout<<endl;
           fprintf(stderr, "\tError : %s", globus_object_printable_to_string(err));
           done = GLOBUS_TRUE;
        }
        else {
           rc = fread(buffer,1,MAX_BUFFER_SIZE,fd);
           globus_ftp_client_register_write(
                           &handle,
                           buffer,
                           rc,
                           0,
                           feof(fd) != SUCCESS,
                           gridftp_cb::data_cb,
                           (void *) fd);
           }

           /* lock on condition */
           waitOnCond();
            fclose(fd);

        globus_ftp_client_handle_destroy(&handle);
        return 0;
};


/* start of main */
int main(int argc , char **argv) {

    int                     hostCount;
        string             destinationUrl,sourceUrl,tempUrl;
        string             sourceFile,destinationFile,outputFile,tmpFile;

        /* Creating the object */
        GridFTP         gridftp;

        /* Checking for command line arguments */
        if ( argc != 2 ) {

                cout<<endl<<"\t\t ERROR: Missing or Invalid command line arguments";
                cout<<endl<<"\t\t        must specify a <config file> ";
                cout<<endl<<"\t\t Usage: <executable> <config file> "<<endl<<endl;
                exit(0);
        }


       /* Reading input parameters from the configuration file */
        if ( gridftp.readConfigFile(argv[1]) == 1 )
               return 1;

        /* Validate the input parameters */
        if ( gridftp.validateConfigFile(argv[1]) == 1 )
              return 1;


        /* Performing the Gridftp client to server file transfer */
        cout <<endl<<"\t ************* GridFTP client to server file Transfer :: start ***************************"<<endl;


       for ( hostCount = 0 ; hostCount < gridftp.host_count ; hostCount++) {

              /* Assign the source file path */
             sourceFile="./input/gridftpClientToServerTransfer.txt";

                   /* Preparing the source url and destination url */
                   tempUrl.erase();
                   destinationUrl.erase();
                   destinationUrl.operator+=("gsiftp://");
                   destinationUrl.operator+=(gridftp.host_string_only[hostCount]);
                   destinationUrl.operator+=(gridftp.host_wd_string[hostCount]);
                   tempUrl.operator=(destinationUrl);
                   tempUrl.operator+=(sourceFile.substr(8));
              
         cout<<endl<<"\t\t Source file path : "<<sourceFile;
        cout<<endl<<"\t\t Destination Url : "<<tempUrl<<endl;

                  /*Call for function to transfer the data file from client site to remote site */
                   gridftp.gridftpClientToServer(sourceFile,tempUrl);

         }


        cout<<endl<<endl<<"\t *********** GridFTP client to server file Transfer :: completed *****************"<<endl<<endl;

        return 0;

 }/*End of main */

