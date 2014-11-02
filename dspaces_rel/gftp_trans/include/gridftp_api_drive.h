#ifndef _GRIDFTP_DRIVE_H_
#define _GRIDFTP_DRIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globus_ftp_client.h"

struct trans_context {
  FILE *fd;
  globus_bool_t done;
  globus_mutex_t lock;
  globus_cond_t cond;
};

void load_gftp_modules();
void unload_gftp_modules();
int gridftp_get_file(const char* src_url, const char* dst_url);
int gridftp_put_file(const char* src_url, const char* dst_url);
// int gridftp_fancy_put_file(const char* src_url, const char* dst_url, int num_streams);

#endif //end of _GRIDFTP_DRIVE_H_