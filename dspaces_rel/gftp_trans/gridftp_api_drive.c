#include "gridftp_api_drive.h"

#define MAX_BUFFER_SIZE_W 5

//forward declarations
void done_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *err);
void data_write_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t * err, 
                          globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof);
void load_gftp_modules();
void unload_gftp_modules();
void _wait(void* user_arg);
void _continue(void* user_arg);
//
void done_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *err)
{
  if ( err ) {
    printf("done_cb:: \n");
    printf("\t Status= File Transfer Failed \n");
    printf("\t ERROR=%s \n", globus_object_printable_to_string(err) );
  }
  else{
    printf("done_cb:: \n");
    printf("\t Status= File Transferred Successfully \n");
    free((struct trans_context*)user_arg);
  }
  
  _continue(user_arg);
}

void data_read_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t * err, 
                  globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
  if (err) {
    printf("data_read_cb::\n\t Error= %s \n", globus_object_printable_to_string(err) );
  }
  else {
    if (!eof) {
      struct trans_context *tc_ = (struct trans_context*)user_arg;
      FILE *fd = tc_->fd;
      globus_mutex_lock(&tc_->lock);
      unsigned long int rc = fwrite(buffer, 1, MAX_BUFFER_SIZE_W, fd);
      globus_mutex_unlock(&tc_->lock);
      if ( ferror(fd) != 0) {
        printf("data_read_cb::\n\t Error= %d", errno);
        return;
      }
      globus_ftp_client_register_read(handle, buffer, MAX_BUFFER_SIZE_W, data_read_cb, (void*) tc_);
    }
    else {
      // globus_libc_free(buffer);
    }
  }
}

void data_write_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t * err, 
                   globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
  if (err) {
    printf("data_write_cb::\n\t Error= %s \n", globus_object_printable_to_string(err) );
  }
  else {
    if (!eof) {
      struct trans_context *tc_ = (struct trans_context*)user_arg;
      FILE *fd = tc_->fd;
      globus_mutex_lock(&tc_->lock);
      unsigned long int rc = fread(buffer, 1, MAX_BUFFER_SIZE_W, fd);
      globus_mutex_unlock(&tc_->lock);
      if ( ferror(fd) != 0) {
        printf("data_write_cb::\n\t Error= %d", errno);
        return;
      }
      globus_ftp_client_register_write(handle, buffer, rc, offset+length, feof(fd) != 0, data_write_cb, (void*) tc_);
    }
    else {
      // globus_libc_free(buffer);
    }
  }
}

void load_gftp_modules()
{
  globus_result_t status = (globus_result_t)globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
  if (status != GLOBUS_SUCCESS) {
    char* tmpstr = globus_object_printable_to_string(globus_error_get(status) );
    printf("Failed to load GLOBUS_FTP_CLIENT_MODULE. Error=%s - %s \n", status, tmpstr);
    exit(1);
  }
  
  //
  printf("load_gftp_modules:: loaded. \n");
}

void unload_gftp_modules()
{
  globus_module_deactivate_all();
  
  //
  printf("load_gftp_modules:: unloaded. \n");
}

void _wait(void* user_arg)
{
  struct trans_context* tc_ = (struct trans_context*)user_arg;
  globus_mutex_lock(&tc_->lock);
  while (!tc_->done) {
    globus_cond_wait(&tc_->cond, &tc_->lock);
  }
  globus_mutex_unlock(&tc_->lock);
}

void _continue(void *user_arg)
{
  struct trans_context* tc_ = (struct trans_context*)user_arg;
  globus_mutex_lock(&tc_->lock);
  tc_->done = GLOBUS_TRUE;
  globus_cond_signal(&tc_->cond);
  globus_mutex_unlock(&tc_->lock);
}


int gridftp_get_file(const char* src_url, const char* dst_url)
{
  struct trans_context *tc_ = (struct trans_context*)malloc(sizeof(struct trans_context) );
  tc_->done = GLOBUS_FALSE;
  // 
  load_gftp_modules();
  
  globus_ftp_client_handleattr_t            hattr;
  globus_ftp_client_operationattr_t         oattr;
  globus_ftp_client_handle_t                handle;
  globus_byte_t                             buffer[MAX_BUFFER_SIZE_W];
  globus_result_t                           status;
  
  //Initialize the handle attribute
  if (globus_ftp_client_handleattr_init(&hattr) != GLOBUS_SUCCESS) {
    printf("gridftp_get_file::\n\t ERROR= Failed to activate the ftp client handleattr \n");
    return 1;
  }
  //Initialize the operation attribute
  if (globus_ftp_client_operationattr_init(&oattr) != GLOBUS_SUCCESS) {
    printf("gridftp_get_file::\n\t ERROR= Failed to initialize operationattr \n");
    return 1;
  }
  //Initalize the handle
  if (globus_ftp_client_handle_init(&handle, &hattr) != GLOBUS_SUCCESS) {
    printf("gridftp_get_file::\n\t ERROR= Failed to initialize the handle \n");
    return 1;
  }
  //
  FILE *fd = fopen(dst_url,"w");
  if (fd == NULL) {
    printf("gridftp_get_file::\n\t ERROR= Failed to open dst_url= %s \n");
    free(tc_);
    return 1;
  }
  tc_->fd = fd;
  // 
  status = globus_ftp_client_get(&handle, src_url, GLOBUS_NULL, GLOBUS_NULL, done_cb, (void*)tc_);
  if (status != GLOBUS_SUCCESS) {
    globus_object_t* err = globus_error_get(status);
    printf("gridftp_get_file::\n\t Error= %s \n", globus_object_printable_to_string(err) );
    return 1;
  }
  else {
    status = globus_ftp_client_register_read(&handle, buffer, MAX_BUFFER_SIZE_W, data_read_cb, (void*) tc_);
    if (status != GLOBUS_SUCCESS) {
      printf("gridftp_get_file:: globus_ftp_client_register_get failed; status= %s \n", status);
      return 1;
    }
  }
  
  _wait((void*)tc_);
  //
  fclose(fd);
  globus_ftp_client_handle_destroy(&handle);
  unload_gftp_modules();
  //
  return 0;
}

int gridftp_put_file(const char* src_url, const char* dst_url)
{
  struct trans_context *tc_ = (struct trans_context*)malloc(sizeof(struct trans_context) );
  tc_->done = GLOBUS_FALSE;
  // 
  load_gftp_modules();
  
  globus_ftp_client_handleattr_t            hattr;
  globus_ftp_client_operationattr_t         oattr;
  globus_ftp_client_handle_t                handle;
  globus_byte_t                             buffer[MAX_BUFFER_SIZE_W];
  globus_result_t                           status;
  
  //Initialize the handle attribute
  if (globus_ftp_client_handleattr_init(&hattr) != GLOBUS_SUCCESS) {
    printf("gridftp_put_file::\n\t ERROR= Failed to activate the ftp client handleattr \n");
    return 1;
  }
  //Initialize the operation attribute
  if (globus_ftp_client_operationattr_init(&oattr) != GLOBUS_SUCCESS) {
    printf("gridftp_put_file::\n\t ERROR= Failed to initialize operationattr \n");
    return 1;
  }
  //Initalize the handle
  if (globus_ftp_client_handle_init(&handle, &hattr) != GLOBUS_SUCCESS) {
    printf("gridftp_put_file::\n\t ERROR= Failed to initialize the handle \n");
    return 1;
  }
  //
  FILE *fd = fopen(src_url,"r");
  if (fd == NULL) {
    printf("gridftp_put_file::\n\t ERROR= Failed to open src_url= %s \n");
    free(tc_);
    return 1;
  }
  tc_->fd = fd;
  // 
  status = globus_ftp_client_put(&handle, dst_url, GLOBUS_NULL, GLOBUS_NULL, done_cb, (void*)tc_);
  if (status != GLOBUS_SUCCESS) {
    globus_object_t* err = globus_error_get(status);
    printf("gridftp_put_file::\n\t Error= %s \n", globus_object_printable_to_string(err) );
    return 1;
  }
  else {
    unsigned long int rc = fread(buffer, 1, MAX_BUFFER_SIZE_W, tc_->fd);
    status = globus_ftp_client_register_write(&handle, buffer, rc, 0, feof(fd) != 0, data_write_cb, (void*) tc_);
    if (status != GLOBUS_SUCCESS) {
      printf("gridftp_put_file:: globus_ftp_client_register_write failed; status= %s \n", status);
      return 1;
    }
  }
  
  _wait((void*)tc_);
  //
  fclose(fd);
  globus_ftp_client_handle_destroy(&handle);
  unload_gftp_modules();
  //
  return 0;
}

// int gridftp_fancy_put_file(const char* src_url, const char* dst_url, int num_streams)
// {
//   globus_ftp_client_handle_t handle;
//   globus_ftp_client_operationattr_t attr;
//   globus_ftp_client_handleattr_t handle_attr;
//   globus_ftp_control_parallelism_t parallelism;
//   globus_ftp_control_layout_t layout;
//   globus_byte_t* buffer;
//   globus_result_t status;
//   globus_ftp_client_restart_marker_t restart;
//   globus_ftp_control_type_t filetype;
//   globus_ftp_control_tcpbuffer_t tcpbuffer;
  
//   globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
//   globus_mutex_init(&lock, GLOBUS_NULL);
//   globus_cond_init(&cond, GLOBUS_NULL);
//   globus_ftp_client_handle_init(&handle,  GLOBUS_NULL);
//   globus_ftp_client_handleattr_init(&handle_attr);
//   globus_ftp_client_operationattr_init(&attr);
//   layout.mode = GLOBUS_FTP_CONTROL_STRIPING_NONE;
//   globus_ftp_client_restart_marker_init(&restart);
//   globus_ftp_client_operationattr_set_mode(&attr, GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
  
//   if (num_streams >= 1){
//     parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
//     parallelism.fixed.size = num_streams;
//     globus_ftp_client_operationattr_set_parallelism(&attr, &parallelism);
//   }
  
//   globus_ftp_client_operationattr_set_layout(&attr, &layout);
  
//   filetype = GLOBUS_FTP_CONTROL_TYPE_IMAGE;
//   globus_ftp_client_operationattr_set_type (&attr, filetype);
//   //Use large TCP windows for WANs with latency
//   tcpbuffer.mode =  GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
//   tcpbuffer.fixed.size = (1024 * 1024 * 16);
  
//   globus_ftp_client_operationattr_set_tcp_buffer(&attr, &tcpbuffer);
//   globus_ftp_client_handle_init(&handle, &handle_attr);
//   //
//   done = GLOBUS_FALSE;
  
//   FILE *fd = fopen(src_url,"r");
//   if (fd == NULL){
//     printf("gridftp_fancy_put_file::\n\t ERROR= Failed to open src_url= %s \n");
//     return 1;
//   }
  
//   status = globus_ftp_client_put(&handle, dst_url, GLOBUS_NULL, GLOBUS_NULL, done_cb, 0);
//   if ( status != GLOBUS_SUCCESS ) {
//     globus_object_t* err = globus_error_get(status);
//     printf("gridftp_fancy_put_file::\n\t ERROR= %s \n", globus_object_printable_to_string(err) );
//     return 1;
//   }
//   else {
//     unsigned long int rc, curr_offset;
//     int i;
//     for (i = 0; i < 2*num_streams && feof(fd) == 0; i++){
//       buffer = malloc(MAX_BUFFER_SIZE_W);
//       globus_mutex_lock(&lock);
//       rc = fread(buffer, 1, MAX_BUFFER_SIZE_W, fd);
//       globus_mutex_unlock(&lock);
//       status = globus_ftp_client_register_write(&handle, buffer, rc, curr_offset, feof(fd) != 0, data_write_cb, (void*) fd);
//       if (status != GLOBUS_SUCCESS){
//         printf("gridftp_fancy_put_file:: globus_ftp_client_register_write failed; status= %s \n", status);
//         return 1;
//       }
//     }
//   }
  
//   _wait(user_arg);
//   //
//   fclose(fd);
//   globus_ftp_client_handle_destroy(&handle);
//   unload_gftp_modules();
  
//   //
//   return 0;
// }
