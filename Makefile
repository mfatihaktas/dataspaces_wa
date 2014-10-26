# DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# BOOSTASIO_DIR = /opt/matlab/R2013a/bin/glnxa64
# DSPACES_WA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
DATASPACES_INC = -I$(DATASPACES_DIR)/include
DATASPACES_LIB = -L$(DATASPACES_DIR)/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -l:libglog.a

# GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
# GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so
# 
DSPACES_REL_DIR = dspaces_rel
DSPACES_REL_ODIR = $(DSPACES_REL_DIR)/obj

OVERLAYNET_DIR = dspaces_rel/overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

IBTRANS_DIR = dspaces_rel/ib_trans
IBTRANS_ODIR = $(IBTRANS_DIR)/obj

DATASPACESWA_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa
DATASPACESWA_INC = -I$(DATASPACESWA_DIR)/include -I$(DATASPACESWA_DIR)/$(DSPACES_REL_DIR)/include -I$(DATASPACESWA_DIR)/$(OVERLAYNET_DIR)/include -I$(DATASPACESWA_DIR)/$(IBTRANS_DIR)/include
DATASPACESWA_LIB = $(DATASPACESWA_DIR)/lib

IBVERBS_LIB = -L/usr/lib64 -libverbs

# MPI_DIR ?= /cac/u01/mfa51/Desktop/mpich-3.1.2/install
# MPI_LIB = -L$(MPI_DIR)/lib -lmpi -lmpicxx -lmpichf90
# 
INC = $(DATASPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DATASPACESWA_INC)
LIB = $(DATASPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB)

IDIR = include
ODIR = obj
LIBDIR = lib

.PHONY: lib all lclean clean submake_dspaces_rel

APPS := exp

all: submake_dspaces_rel ${APPS}

submake_dspaces_rel:
	make -C $(DSPACES_REL_DIR)

exp: $(ODIR)/exp.o $(ODIR)/dataspaces_wa.o $(DSPACES_REL_ODIR)/ds_client.o $(DSPACES_REL_ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -g -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

lib: 
	ar -cvq $(DATASPACESWA_LIB)/libdspaces_wa.a $(ODIR)/dataspaces_wa.o $(DSPACES_REL_ODIR)/ds_client.o $(DSPACES_REL_ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o

lclean:
	rm -f $(ODIR)/*.o

clean:
	make -C $(DSPACES_REL_DIR) clean
	rm -f $(ODIR)/*.o $(LIBDIR)/* ${APPS}