# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# GFTPINC_DIR ?= 
# GFTPLIB_DIR ?= 
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

ifndef MPI_DIR
	MPI_LIB = 
	# $(error MPI_DIR is undefined)
else
	MPI_LIB = -L$(MPI_DIR)/lib
endif
# MPI_LIB = -L$(MPI_DIR)/lib -lmpich -lmpichcxx
# 
DSPACESREL_DIR = $(DSPACESWA_DIR)/dspaces_rel
DSPACESREL_ODIR = $(DSPACESREL_DIR)/obj
DSPACESREL_OBJS = $(DSPACESREL_ODIR)/ds_client.o $(DSPACESREL_ODIR)/ds_drive.o

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

PREFETCH_DIR = $(DSPACESREL_DIR)/prefetch
PREFETCH_ODIR = $(PREFETCH_DIR)/obj
PREFETCH_OBJS = $(PREFETCH_ODIR)/prefetch.o $(PREFETCH_ODIR)/palgorithm.o

DSPACESWA_INC = -I$(DSPACESWA_DIR)/include -I$(DSPACESREL_DIR)/include -I$(OVERLAYNET_DIR)/include -I$(IBTRANS_DIR)/include -I$(GFTPTRANS_DIR)/include -I$(PREFETCH_DIR)/include
DSPACESWA_LIB = $(DSPACESWA_DIR)/lib

IBVERBS_LIB = -L/usr/lib64 -libverbs
# 
INC = $(DSPACES_INC) $(GLOG_INC) $(BOOST_INC) $(DSPACESWA_INC) $(GFTP_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB) $(GFTP_LIB)

ODIR = obj
OBJS = $(ODIR)/dataspaces_wa.o $(DSPACESREL_OBJS) $(OVERLAYNET_OBJS) $(IBTRANS_OBJS) $(GFTPTRANS_OBJS) $(PREFETCH_OBJS)
LIBDIR = lib

.PHONY: lib all lclean clean submake_dspaces_rel

APPS := exp ds_wa_test

all: submake_dspaces_rel ${APPS}

submake_dspaces_rel:
	make -C $(DSPACESREL_DIR)

ds_wa_test: $(ODIR)/ds_wa_test.o $(OBJS)
	$(MPICPP) $(MPICPP_OPTS) -g -o $@ $^ $(INC) $(LIB)
	
exp: $(ODIR)/exp.o $(OBJS)
	$(MPICPP) $(MPICPP_OPTS) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) $(MPICPP_OPTS) -g -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

lib: 
	ar -cvq $(DSPACESWA_LIB)/libdspaces_wa.a $(OBJS)

lclean:
	rm -f $(ODIR)/*.o

clean:
	make -C $(DSPACESREL_DIR) clean
	rm -f $(ODIR)/*.o $(LIBDIR)/* ${APPS}