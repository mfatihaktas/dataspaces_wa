#include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 //#include "globus_gass_copy.h"
 #include "globus_ftp_client.h"
 #include <sys/stat.h>
 #include <time.h>
 #include <stddef.h>
 
 #define PORT 8888
 #define FILENAME "/tmp/messages"
 #define MIMETYPE "text/plain"
 #define PARALLELISM 4 
 #define array_size 300
 #define array_http_size 4067
 #define BUFFER_SIZE 1024*1024
 #define remote_host "127.0.0.1:5000"
 #define get_file "/cac/u01/mfa51/Desktop/dataspaces_wa/transfer/dummy_.dat"
 #define curr_wr_dir "/cac/u01/mfa51/Desktop/dataspaces_wa/transfer/"
 #define MAXLEN 1200
 #define MAX_BUFFER_SIZE_R 1048576
 #define MAX_BUFFER_SIZE_W 16777216
 
//1048576
 unsigned long int  global_offset=0;
 int t = 0;
 static globus_mutex_t lock;
 static globus_cond_t cond;
 static globus_bool_t done;
 char get_outfile[array_size];
 globus_byte_t* from_gridftp_2_http[array_http_size];
 globus_off_t off[array_http_size];
static void done_cb( void *         user_arg,
                     globus_ftp_client_handle_t *   handle,
                     globus_object_t *       err)
{

    if(err){
        fprintf(stderr, "%s", globus_object_printable_to_string(err));
    }
    
    globus_mutex_lock(&lock);
    done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
    globus_mutex_unlock(&lock);
    return;
}

static void data_cb_read( void *       user_arg,
                          globus_ftp_client_handle_t *   handle,
                          globus_object_t *     err,
                          globus_byte_t *     buffer,
                          globus_size_t      length,
                          globus_off_t       offset,
                          globus_bool_t     eof)

{ 
    if(err) {
        fprintf(stderr, "%s", globus_object_printable_to_string(err));
    }
    else {
       FILE* fd = (FILE*) user_arg;
       globus_assert(fd);
       unsigned long int  rc = fwrite(buffer, 1,length, fd);
                     if (ferror(fd)) {
            printf("Read error in function data_cb_read; errno = %d\n", errno);
            return;
        }

        if (!eof) {
            globus_ftp_client_register_read(handle,
                                            buffer,
                                            MAX_BUFFER_SIZE_R,
                                            data_cb_read,
                                            (void*) fd);
printf("[%"GLOBUS_OFF_T_FORMAT",%ld]\n",offset,length);      
 }
    }

    return;
}

static void data_cb_write( void *       user_arg,
                           globus_ftp_client_handle_t * handle,
                           globus_object_t *     err,
                           globus_byte_t *     buffer,
                           globus_size_t     length,
                           globus_off_t     offset,
                           globus_bool_t     eof)
{
    unsigned long int curr_offset;
    if(err) {
        fprintf(stderr, "%s", globus_object_printable_to_string(err));
    }
    else {
        if (!eof) {
            FILE* fd = (FILE*) user_arg;
            unsigned long int rc;
            globus_mutex_lock(&lock);
            curr_offset = global_offset;
            rc = fread(buffer, 1, MAX_BUFFER_SIZE_W, fd);
            global_offset += rc;
            globus_mutex_unlock(&lock);
            if (ferror(fd)) {
                printf("Read error in function data_cb_write; errno = %d\n", errno);
                return;
            }

            globus_ftp_client_register_write(handle,
                                             buffer,
               rc,
               curr_offset,
                                             feof(fd) != 0,
                                             data_cb_write,
                                             (void*) fd);
 printf("[%"GLOBUS_OFF_T_FORMAT",%ld,%ld,%ld]\n",offset,length,global_offset,curr_offset);
        } else {
            globus_libc_free(buffer);
        }
    }
    return;
}

void get_file_data_cb(void* user_arg ,globus_ftp_client_handle_t* handle ,
                          globus_object_t* err , globus_byte_t*  buffer_t,
                          globus_size_t  length , globus_off_t offset ,
                          globus_bool_t eof)
   {
if(err)
{
     fprintf(stderr, "%s", globus_object_printable_to_string(err));
}
else
{

 if(!eof)
    {
int f1,f2;
f1 = offset;
f2 = length;
t = f1/f2;
off[t] = offset;
from_gridftp_2_http[t] = buffer_t;
globus_ftp_client_register_read(
            handle,
            buffer_t,
            BUFFER_SIZE,
            get_file_data_cb,
            0);
printf("[%"GLOBUS_OFF_T_FORMAT",%ld,%c,%c,%i,%"GLOBUS_OFF_T_FORMAT"]\n",offset,length,*buffer_t,*from_gridftp_2_http[t],t,off[t]);
}
}
}


int gridftp_put(char* src_url, char* dst_url, FILE **gsiftpfile, int num_streams)
{
    int i;
    globus_ftp_client_handle_t     handle;
    globus_ftp_client_operationattr_t   attr;
    globus_ftp_client_handleattr_t   handle_attr;
    globus_ftp_control_parallelism_t     parallelism;
    globus_ftp_control_layout_t    layout;
    globus_byte_t *       buffer;
    globus_size_t buffer_length = sizeof(buffer);
    globus_result_t       result;
    globus_ftp_client_restart_marker_t  restart;
    globus_ftp_control_type_t     filetype;
    globus_ftp_control_tcpbuffer_t  tcpbuffer;
   
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);
    globus_ftp_client_handle_init(&handle,  GLOBUS_NULL);
    globus_ftp_client_handleattr_init(&handle_attr);
    globus_ftp_client_operationattr_init(&attr);
    layout.mode = GLOBUS_FTP_CONTROL_STRIPING_NONE;
    globus_ftp_client_restart_marker_init(&restart);
    globus_ftp_client_operationattr_set_mode(
            &attr,
            GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
   
    if (num_streams >= 1)
    {
        parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
        parallelism.fixed.size = num_streams;
       
        globus_ftp_client_operationattr_set_parallelism(
            &attr,
            &parallelism);
    }
   
    globus_ftp_client_operationattr_set_layout(&attr,
                                               &layout);
   
    filetype = GLOBUS_FTP_CONTROL_TYPE_IMAGE;
    globus_ftp_client_operationattr_set_type (&attr,
                                              filetype);
   /* use large TCP windows for WANs with latency */
    tcpbuffer.mode =  GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
    tcpbuffer.fixed.size = (1024 * 1024 * 16);

    globus_ftp_client_operationattr_set_tcp_buffer(&attr, &tcpbuffer);
   
    globus_ftp_client_handle_init(&handle, &handle_attr);
    
    done = GLOBUS_FALSE;
    //strcpy(get_outfile, curr_wr_dir);
    //strcat(get_outfile,"nest_p400__tcpbs0111112.gz");
    //*gsiftpfile = fopen(get_outfile,"r");  
    *gsiftpfile = fopen(src_url,"r");
   
   
    result = globus_ftp_client_put(&handle,c
                                   dst_url,
                                   &attr,
                                   &restart,
                                   done_cb,
                                   0);
    if(result != GLOBUS_SUCCESS) {
        globus_object_t * err;
        err = globus_error_get(result);
        fprintf(stderr, "%s", c(err));
        done = GLOBUS_TRUE;
    }
    else {
       unsigned long int rc;
       unsigned long int curr_offset;

  for (i = 0; i< 2 * num_streams && feof(*gsiftpfile) == 0; i++)
        {
            buffer = malloc(MAX_BUFFER_SIZE_W);
            globus_mutex_lock(&lock);
            curr_offset = global_offset;
            rc = fread(buffer, 1, MAX_BUFFER_SIZE_W, *gsiftpfile);
            global_offset += rc;
            globus_mutex_unlock(&lock);
            globus_ftp_client_register_write(
                &handle,
                buffer,
                rc,
                curr_offset,
                feof(*gsiftpfile) != 0,
                data_cb_write,
                (void*) *gsiftpfile);
        }
    }
   
    globus_mutex_lock(&lock);

    while(!done) {
        globus_cond_wait(&cond, &lock);
    }

    globus_mutex_unlock(&lock);
    globus_ftp_client_handle_destroy(&handle);
    globus_module_deactivate_all();
   
    return 0;
}

/*
int grid_ftp_get_server_file(char *dst_url,FILE **gsiftpfile,int num_streams)
 {
int i;
globus_ftp_client_handle_t     handle;
    globus_ftp_client_operationattr_t   attr;
    globus_ftp_client_handleattr_t   handle_attr;
    globus_ftp_control_parallelism_t     parallelism;
    globus_ftp_control_layout_t    layout;
    globus_byte_t       buffer[MAX_BUFFER_SIZE_R];
    globus_size_t                       block_size;
    globus_size_t buffer_length = sizeof(buffer);
    globus_result_t       result;
    globus_ftp_client_restart_marker_t  restart;
    globus_ftp_control_type_t     filetype;
    globus_ftp_control_tcpbuffer_t      tcpbuffer;
    globus_gass_copy_handle_t *          gass_copy_handle;
    globus_gass_copy_handleattr_t       gass_copy_handleattr;   
 
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);
    globus_ftp_client_handle_init(&handle,  GLOBUS_NULL);
    globus_gass_copy_handleattr_init(&gass_copy_handleattr);
    globus_ftp_client_handleattr_init(&handle_attr);
    globus_ftp_client_operationattr_init(&attr);
    layout.mode = GLOBUS_FTP_CONTROL_STRIPING_NONE;
    globus_ftp_client_restart_marker_init(&restart);
    globus_ftp_client_operationattr_set_mode(
            &attr,
            GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
   block_size = MAX_BUFFER_SIZE_R;
    // block_size = 1024;
     globus_ftp_client_operationattr_set_type (&attr,
                                              filetype);
    globus_gass_copy_handleattr_set_ftp_attr(&gass_copy_handleattr,&handle_attr);
    globus_gass_copy_handle_init(&gass_copy_handle, &gass_copy_handleattr);
    globus_gass_copy_set_buffer_length(&gass_copy_handle,block_size);
    printf("[%ld]\n",block_size);
 
    if (num_streams >= 1)
    {
        parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
        parallelism.fixed.size = num_streams;
       
        globus_ftp_client_operationattr_set_parallelism(
            &attr,
            &parallelism);
    }
    
    globus_ftp_client_operationattr_set_layout(&attr,
                                               &layout);
   
    filetype = GLOBUS_FTP_CONTROL_TYPE_IMAGE;
    globus_ftp_client_operationattr_set_type (&attr,
                                              filetype);
    //use large TCP windows for WANs with latency
    tcpbuffer.mode =  GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
    tcpbuffer.fixed.size = (1024 * 1024 * 16);

    globus_ftp_client_operationattr_set_tcp_buffer(&attr, &tcpbuffer);

//   printf("[%ld]\n",tcpbuffer);  

    globus_ftp_client_handle_init(&handle, &handle_attr);
    
   done = GLOBUS_FALSE;
   strcpy(get_outfile, curr_wr_dir);
   strcat(get_outfile,"nest_p400__tcpbs0111112.gz");
   *gsiftpfile = fopen(get_outfile,"w+");

  result = globus_ftp_client_get(&handle,
                                   dst_url,
                                   &attr,
                                   &restart,
                                   done_cb,
                                   0);
    if(result != GLOBUS_SUCCESS) {
        globus_object_t * err;
        err = globus_error_get(result);
        fprintf(stderr, "%s", globus_object_printable_to_string(err));
        done = GLOBUS_TRUE;
}
 else {
       
 globus_ftp_client_register_read(&handle,
                                        buffer,
                                        buffer_length,
                                        data_cb_read,
                                        (void*) *gsiftpfile);
    }

    globus_mutex_lock(&lock);

    while(!done) {
        globus_cond_wait(&cond, &lock);
    }

    globus_mutex_unlock(&lock);
    globus_gass_copy_handleattr_destroy(&gass_copy_handleattr);
    globus_ftp_client_handle_destroy(&handle);
    globus_module_deactivate_all();
   
    return 0;
   
}
*/

 int main(int argc, char *argv[])
 {
  FILE *gsiftpfile;
  char src_url[array_size];
  char dst_url[array_size];
  int num_streams = 10;
  
  //printf("\n\t\t DESCRIPTION : Program to get the file fron the remote server to client site using GridFTP\n");


  //Prepare the url to get the contents of the file
  strcpy(src_url, "/cac/u01/mfa51/Desktop/dataspaces_wa/transfer/dummy.dat");
  
  strcpy(dst_url, "ftp://");
  strcat(dst_url, remote_host);
  strcat(dst_url, get_file);

  //strcpy(get_outfile, curr_wr_dir);
  //strcat(get_outfile,"messages");
  //globus_libc_printf("\n\t\t -->  file %s from %s",get_file,remote_host);
  //printf(" \n\t\t and writting into : %s ",get_outfile);


  // Getting the contents of the file from Remote sites
  //fd = fopen(get_outfile,"w");
  //grid_ftp_get_server_file(dst_url,&gsiftpfile,num_streams);
  printf("src_url= %s\n", src_url);
  printf("dst_url= %s\n", dst_url);
  
  gridftp_put(src_url, dst_url,&gsiftpfile,num_streams);
 return 0;
 }