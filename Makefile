DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0
DATASPACES_INC = -I$(DATASPACES_DIR)/install/include
DATASPACES_LIB = -L$(DATASPACES_DIR)/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so

DATASPACESWA_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa
SUB_INC = -I$(DATASPACESWA_DIR)/include -I$(DATASPACESWA_DIR)/dspaces_rel/include -I$(DATASPACESWA_DIR)/dspaces_rel/overlaynet/include
#
INC = $(DATASPACES_INC) $(GLOG_INC) $(SUB_INC)
LIB = $(DATASPACES_LIB) $(GLOG_LIB) $(BOOST_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = gcc
CPP = g++

IDIR = include
ODIR = obj

DSPACES_REL_DIR = dspaces_rel
DSPACES_REL_ODIR = $(DSPACES_REL_DIR)/obj

OVERLAYNET_DIR = dspaces_rel/overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

.PHONY: all clean submake_dspaces_rel

all: submake_dspaces_rel exp

submake_dspaces_rel:
	make -C $(DSPACES_REL_DIR)

exp: $(ODIR)/exp.o $(ODIR)/dataspaces_wa.o $(DSPACES_REL_ODIR)/ds_client.o $(DSPACES_REL_ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

clean:
	make -C $(DSPACES_REL_DIR) clean
	rm -f $(ODIR)/*.o