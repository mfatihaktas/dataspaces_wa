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
# ###
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so
# 
GFTPTRANS_INC = -I$(DSPACESWA_DIR)/gftp_trans/include

INC = $(GFTPTRANS_INC) $(GFTP_INC) $(GLOG_INC) $(BOOST_INC)
LIB = $(GFTP_LIB) $(GLOG_LIB) $(BOOST_LIB)
# 
IDIR = include
ODIR = obj

.PHONY: clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/gftp_delivery.o $(ODIR)/gftp_drive.o $(ODIR)/gridftp_api_drive.o $(ODIR)/io_drive.o
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