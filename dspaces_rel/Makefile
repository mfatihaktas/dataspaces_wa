# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
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

ifndef MPI_DIR
	MPI_LIB = 
	# $(error MPI_DIR is undefined)
else
	MPI_LIB = -L$(MPI_DIR)/lib
endif
# MPI_LIB = -L$(MPI_DIR)/lib -lmpich -lmpichcxx

IBVERBS_LIB = -L/usr/lib64 -libverbs
# 
OVERLAYNET_DIR = overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj

IBTRANS_DIR = ib_trans
IBTRANS_ODIR = $(IBTRANS_DIR)/obj

DSPACESREL_DIR=$(DSPACESWA_DIR)/dspaces_rel
DSPACESREL_INC = -I$(DSPACESREL_DIR)/include -I$(DSPACESREL_DIR)/$(OVERLAYNET_DIR)/include -I$(DSPACESREL_DIR)/$(IBTRANS_DIR)/include
# 
INC = $(DSPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DSPACESREL_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB)

IDIR = include
ODIR = obj

.PHONY: all lclean clean submake_ibtrans_dir submake_overlaynet_dir

APPS := exp

all: submake_overlaynet_dir submake_ibtrans_dir ${APPS}

submake_overlaynet_dir:
	make -C $(OVERLAYNET_DIR)

submake_ibtrans_dir:
	make -C $(IBTRANS_DIR)

exp: $(ODIR)/exp.o $(ODIR)/ds_client.o $(ODIR)/ds_drive.o $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -g -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	make -C $(OVERLAYNET_DIR) clean
	make -C $(IBTRANS_DIR) clean
	rm -f $(ODIR)/*.o ${APPS} 