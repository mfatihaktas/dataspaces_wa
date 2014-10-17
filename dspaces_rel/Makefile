DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0
DATASPACES_INC = -I$(DATASPACES_DIR)/install/include
DATASPACES_LIB = -L$(DATASPACES_DIR)/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so

IBVERBS_LIB = -L/usr/lib64 -libverbs

MPI_DIR ?= /cac/u01/mfa51/Desktop/mpich-3.1.2/install
MPI_LIB = -L$(MPI_DIR)/lib -lmpi -lmpicxx -lmpichf90
# 
OVERLAYNET_DIR = overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

IBTRANS_DIR = ib_trans
IBTRANS_ODIR = $(IBTRANS_DIR)/obj

DSPACES_REL_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel
DSPACES_REL_INC = -I$(DSPACES_REL_DIR)/include -I$(DSPACES_REL_DIR)/$(OVERLAYNET_DIR)/include -I$(DSPACES_REL_DIR)/$(IBTRANS_DIR)/include
# 
INC = $(DATASPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DSPACES_REL_INC)
LIB = $(DATASPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB)

MPICC = mpicc
# MPICPP = mpic++
MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
MPICPPOPTS = 
CC = /opt/gcc-4.8.2/bin/gcc
CPP = /opt/gcc-4.8.2/bin/g++

IDIR = include
ODIR = obj

.PHONY: all lclean clean submake_ibtrans_dir submake_overlaynet_dir

APPS := exp

all: submake_overlaynet_dir submake_ibtrans_dir ${APPS}
# dummy2 dummy_get dummy_put

submake_overlaynet_dir:
	make -C $(OVERLAYNET_DIR)

submake_ibtrans_dir:
	make -C $(IBTRANS_DIR)

exp: $(ODIR)/exp.o $(ODIR)/ds_client.o $(ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

dummy_put: $(ODIR)/dummy_put.o
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

dummy_get: $(ODIR)/dummy_get.o
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -g -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)

endif

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	make -C $(OVERLAYNET_DIR) clean
	make -C $(IBTRANS_DIR) clean
	rm -f $(ODIR)/*.o ${APPS} 