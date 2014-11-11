# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# GFTPINC_DIR ?= 
# GFTPLIB_DIR ?= 
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
DSPACES_INC = -I$(DSPACES_DIR)/include
DSPACES_LIB = -L$(DSPACES_DIR)/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

ifndef MPI_DIR
	MPI_LIB = 
	# $(error MPI_DIR is undefined)
else
	MPI_LIB = -L$(MPI_DIR)/lib
endif
# MPI_LIB = -L$(MPI_DIR)/lib -lmpich -lmpichcxx

IBVERBS_LIB = -L/usr/lib64 -libverbs
# 
DSPACESREL_DIR=$(DSPACESWA_DIR)/dspaces_rel

OVERLAYNET_DIR = $(DSPACESREL_DIR)/overlaynet
OVERLAYNET_ODIR = $(OVERLAYNET_DIR)/obj
OVERLAYNET_OBJS = $(OVERLAYNET_ODIR)/dht_node.o $(OVERLAYNET_ODIR)/dht_server.o $(OVERLAYNET_ODIR)/dht_client.o $(OVERLAYNET_ODIR)/packet.o

IBTRANS_DIR = $(DSPACESREL_DIR)/ib_trans
IBTRANS_ODIR = $(IBTRANS_DIR)/obj
IBTRANS_OBJS = $(IBTRANS_ODIR)/ib_delivery.o $(IBTRANS_ODIR)/common.o

ifdef GRIDFTP
GFTP_INC = -I$(GFTPINC_DIR)
GFTP_LIB = -L$(GFTPLIB_DIR) -lglobus_ftp_client
GFTPTRANS_DIR = $(DSPACESREL_DIR)/gftp_trans
GFTPTRANS_ODIR = $(GFTPTRANS_DIR)/obj
GFTPTRANS_OBJS = $(GFTPTRANS_ODIR)/gftp_delivery.o $(GFTPTRANS_ODIR)/gftp_drive.o $(GFTPTRANS_ODIR)/gridftp_api_drive.o $(GFTPTRANS_ODIR)/io_drive.o
endif

DSPACESREL_INC = -I$(DSPACESREL_DIR)/include -I$(OVERLAYNET_DIR)/include -I$(IBTRANS_DIR)/include -I$(GFTPTRANS_DIR)/include
# 
INC = $(DSPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DSPACESREL_INC) $(GFTP_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB) $(GFTP_LIB)

IDIR = include
ODIR = obj

.PHONY: all lclean clean submake_ibtrans_dir submake_overlaynet_dir submake_gftptrans_dir

APPS := exp

all: submake_overlaynet_dir submake_ibtrans_dir submake_gftptrans_dir ${APPS}

submake_overlaynet_dir:
	make -C $(OVERLAYNET_DIR)

submake_ibtrans_dir:
	make -C $(IBTRANS_DIR) 

submake_gftptrans_dir:
ifdef GRIDFTP 
	make -C $(GFTPTRANS_DIR)
endif

exp: $(ODIR)/exp.o $(ODIR)/ds_client.o $(ODIR)/ds_drive.o $(OVERLAYNET_OBJS) $(IBTRANS_OBJS) $(GFTPTRANS_OBJS)
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -g -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	make -C $(OVERLAYNET_DIR) clean
	make -C $(IBTRANS_DIR) clean
ifdef GRIDFTP
	make -C $(GFTPTRANS_DIR) clean
endif
	rm -f $(ODIR)/*.o ${APPS} 
	