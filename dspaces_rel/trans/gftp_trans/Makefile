# DIR inits for dell cluster
ifndef CC
	CC ?= /opt/gcc-4.8.2/bin/gcc
endif
ifndef CPP
	CPP ?= /opt/gcc-4.8.2/bin/g++
endif
ifndef DSPACESWA_DIR
	DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
endif
ifndef BOOST_DIR
	BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
endif
ifndef GLOG_DIR
	GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
endif
ifndef GFTPINC_DIR
	GFTPINC_DIR ?= /usr/include/globus
endif
ifndef GFTPLIB_DIR
	GFTPLIB_DIR ?= /usr/lib64
endif
# ###
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

GFTP_INC = -I$(GFTPINC_DIR) #-I/usr/lib64/globus/include
GFTP_LIB = -L$(GFTPLIB_DIR) -lglobus_ftp_client

GFTP_TRANS_DIR = $(DSPACESWA_DIR)/dspaces_rel/trans/gftp_trans
# 

INC = $(GFTP_INC) $(GLOG_INC) $(BOOST_INC) -I$(GFTP_TRANS_DIR)/include
LIB = $(GFTP_LIB) $(GLOG_LIB) $(BOOST_LIB)
# 
ODIR = obj
OBJS = $(ODIR)/exp.o $(ODIR)/gftp_trans.o $(ODIR)/gftp_drive.o $(ODIR)/gridftp_api_drive.o $(ODIR)/io_drive.o

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(OBJS)
	$(CPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
gftp_gp: $(ODIR)/gridftpdrv_get_put.o
	$(CC) -o $@ $^ $(INC) $(LIB)

ctos: $(ODIR)/client_to_server.o
	$(CPP) -o $@ $^ $(INC) $(LIB)
endif

clean:
	rm -f $(ODIR)/*.o ${APPS}
