#include <boost/bind.hpp>
#include <boost/thread.hpp>
//
#include <glog/logging.h>

#include <iostream>
#include <string>

#include "globus_ftp_client.h"


#define MAX_BUFFER_SIZE 5

class DSClient
{
  public:
    DSClient();
    ~DSClient();
    int send_file(std::string src, std::string dst);
    
    void _wait();
    void _continue();
  private:
    globus_ftp_client_handleattr_t            hattr;
    globus_ftp_client_operationattr_t         oattr;
    globus_ftp_client_handle_t                handle;
    globus_byte_t                             buffer[MAX_BUFFER_SIZE];
    globus_size_t                             buffer_length;
    globus_result_t                           status;
    char                                      *tmpstr;
    
    boost::mutex m;
    boost::condition_variable cv;
};

namespace gridftp_cb {
  static void done_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *err)
  {
    DSClient* ds_client_ = (DSClient*) user_arg;
    if ( err ){
      LOG(ERROR) << "done_cb::";
      LOG(ERROR) << "\t Status= File Transferred Failed ";
      LOG(ERROR) << "\t ERROR=" << globus_object_printable_to_string(err);
    }
    else{
      LOG(ERROR) << "done_cb::";
      LOG(INFO) << "\t Status= File Transferred Successfully";
    }
    ds_client_->_continue();
  };
  
  static void data_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t * err, 
                      globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
  {
    LOG(INFO) << "data_cb:: called";
    
    if (err){
      LOG(ERROR) << "data_cb::\n\t ERROR=" << globus_object_printable_to_string(err);
    }
    else {
      if ( !eof ) {
        FILE *fd = (FILE *) user_arg;
        int rc;
        rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
        if ( ferror(fd) != 0){
          LOG(ERROR) << "data_cb::\n\t ERROR-CODE=" << errno;
          return;
        }
        globus_ftp_client_register_write(handle, buffer, rc, offset+length, feof(fd) != 0, data_cb, (void*) fd);
      }
    }
  };
}

DSClient::DSClient()
{
  buffer_length = MAX_BUFFER_SIZE;
  status = (globus_result_t)globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
  if (status != GLOBUS_SUCCESS){
    tmpstr = globus_object_printable_to_string(globus_error_get(status));
    LOG(ERROR) << "Failed to load GLOBUS_FTP_CLIENT_MODULE. Error Code=" << status<< " - " << tmpstr;
    exit(1);
  }
  
  //
  LOG(INFO) << "DSClient:: constructed.";
}

DSClient::~DSClient()
{
  globus_module_deactivate_all();
  
  //
  LOG(INFO) << "DSClient:: destructed.";
}

void DSClient::_wait()
{
  LOG(INFO) << "_wait:: waiting...";
  boost::mutex::scoped_lock lock(m);
  cv.wait(lock);
  LOG(INFO) << "_wait:: done.";
}

void DSClient::_continue()
{
  cv.notify_one();
}

int DSClient::send_file(std::string src_url, std::string dst_url)
{
  /* Initialize the handle attribute */
  if (globus_ftp_client_handleattr_init(&hattr) != GLOBUS_SUCCESS) {
    LOG(ERROR) << "send_file::\n\t ERROR= Failed to activate the ftp client handleattr";
    return 1;
  }
  /* Initialize the operation attribute */
  if (globus_ftp_client_operationattr_init(&oattr) != GLOBUS_SUCCESS) {
    LOG(ERROR) << "send_file::\n\t ERROR= Failed to initialize operationattr";
    return 1;
  }
  /* Initalize the handle */
  if (globus_ftp_client_handle_init(&handle, &hattr) != GLOBUS_SUCCESS) {
    LOG(ERROR) << "send_file::\n\t ERROR= Failed to initialize the handle";
    return 1;
  }
  //
  int rc;
  FILE *fd = fopen(src_url.c_str(),"r");
  if ( fd == NULL ) {
    LOG(ERROR) << "send_file::\n\t ERROR= Failed to open src_url=" << src_url;
    return 1;
  }
  
  /* Gridftp API call to start the put operation */
  status = globus_ftp_client_put(&handle, dst_url.c_str(), GLOBUS_NULL, GLOBUS_NULL, gridftp_cb::done_cb, this);
  if ( status != GLOBUS_SUCCESS ) {
    globus_object_t * err;
    err = globus_error_get(status);
    LOG(ERROR) << "send_file::\n\t ERROR= " << globus_object_printable_to_string(err);
    return 1;
  }
  else {
    rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
    
    LOG(INFO) << "buffer= " << buffer;
    LOG(INFO) << "rc= " << rc;
    
    status = globus_ftp_client_register_write(&handle, buffer, rc, 0, feof(fd) != 0, gridftp_cb::data_cb, (void*) fd);
    if ( status != GLOBUS_SUCCESS ) {
      LOG(INFO) << "send_file:: globus_ftp_client_register_write failed; status= " << status;
    }
    else {
      LOG(INFO) << "send_file:: globus_ftp_client_register_write succeded; status= " << status;
    }
  }
  
  _wait();
  //
  fclose(fd);
  globus_ftp_client_handle_destroy(&handle);
  
  return 0;
}