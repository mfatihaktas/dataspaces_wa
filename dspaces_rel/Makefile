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

SUB_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/include -I/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/overlaynet/include
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

OVERLAYNET_DIR = overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

.PHONY: all clean submake_overlaynet_dir

all: submake_overlaynet_dir exp
# dummy2 dummy_get dummy_put

submake_overlaynet_dir:
	make -C $(OVERLAYNET_DIR)

exp: $(ODIR)/exp.o $(ODIR)/ds_client.o $(ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)

endif

clean:
	make -C $(OVERLAYNET_DIR) clean
	rm -f $(ODIR)/*.o