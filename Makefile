# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# MPI_DIR ?= /cac/u01/mfa51/Desktop/mpich-3.1.2/install
# ##################################################################################################
DSPACES_INC = -I$(DSPACES_DIR)/include
DSPACES_LIB = -L$(DSPACES_DIR)/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

# GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
# GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so
# 
DSPACESREL_DIR = dspaces_rel
DSPACESREL_ODIR = $(DSPACESREL_DIR)/obj

OVERLAYNET_DIR = dspaces_rel/overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

IBTRANS_DIR = dspaces_rel/ib_trans
IBTRANS_ODIR = $(IBTRANS_DIR)/obj

DSPACESWA_INC = -I$(DSPACESWA_DIR)/include -I$(DSPACESWA_DIR)/$(DSPACESREL_DIR)/include -I$(DSPACESWA_DIR)/$(OVERLAYNET_DIR)/include -I$(DSPACESWA_DIR)/$(IBTRANS_DIR)/include
DSPACESWA_LIB = $(DSPACESWA_DIR)/lib

IBVERBS_LIB = -L/usr/lib64 -libverbs

ifndef MPI_DIR
	MPI_LIB = 
	# $(error MPI_DIR is undefined)
else
	MPI_LIB = -L$(MPI_DIR)/lib
endif
# MPI_LIB = -L$(MPI_DIR)/lib -lmpich -lmpichcxx
# 
INC = $(DSPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DSPACESWA_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB)

IDIR = include
ODIR = obj
LIBDIR = lib

.PHONY: lib all lclean clean submake_dspaces_rel

APPS := exp

all: submake_dspaces_rel ${APPS}

submake_dspaces_rel:
	make -C $(DSPACESREL_DIR)

exp: $(ODIR)/exp.o $(ODIR)/dataspaces_wa.o $(DSPACESREL_ODIR)/ds_client.o $(DSPACESREL_ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o
	$(MPICPP) $(MPICPP_OPTS) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) $(MPICPP_OPTS) -g -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

lib: 
	ar -cvq $(DSPACESWA_LIB)/libdspaces_wa.a $(ODIR)/dataspaces_wa.o $(DSPACESREL_ODIR)/ds_client.o $(DSPACESREL_ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o

lclean:
	rm -f $(ODIR)/*.o

clean:
	make -C $(DSPACESREL_DIR) clean
	rm -f $(ODIR)/*.o $(LIBDIR)/* ${APPS}