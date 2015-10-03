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

ifdef GRIDFTP
GFTP_INC = -I$(GFTPINC_DIR) #-I/usr/lib64/globus/include
GFTP_LIB = -L$(GFTPLIB_DIR) -lglobus_ftp_client
endif # GRIDFTP

IBVERBS_LIB = -L/usr/lib64 -libverbs

ifndef MPI_DIR
	MPI_LIB = 
	# $(error MPI_DIR is undefined)
else
	MPI_LIB = -L$(MPI_DIR)/lib
endif
# MPI_LIB = -L$(MPI_DIR)/lib -lmpich -lmpichcxx
# 
PREFETCH_DIR = $(SDMCONTROL_DIR)/prefetch
PREFETCH_ODIR = $(PREFETCH_DIR)/obj
PREFETCH_OBJS = $(PREFETCH_ODIR)/prefetch.o $(PREFETCH_ODIR)/markov.o $(PREFETCH_ODIR)/sfc.o $(PREFETCH_ODIR)/hilbert.o

SDMCONTROL_DIR = $(DSPACESREL_DIR)/sdm_control
SDMCONTROL_INC = -I$(SDMCONTROL_DIR)/include -I$(PREFETCH_DIR)/include
SDMCONTROL_ODIR = $(SDMCONTROL_DIR)/obj
SDMCONTROL_OBJS = $(SDMCONTROL_ODIR)/sdm_control.o $(SDMCONTROL_ODIR)/patch_sdm.o $(SDMCONTROL_ODIR)/sdm_node.o $(SDMCONTROL_ODIR)/sdm_server.o $(SDMCONTROL_ODIR)/sdm_client.o $(SDMCONTROL_ODIR)/packet.o $(PREFETCH_OBJS)

IB_TRANS_DIR ?= $(TRANS_DIR)/ib_trans
IB_TRANS_ODIR = $(IB_TRANS_DIR)/obj
IB_TRANS_OBJS = $(IB_TRANS_ODIR)/ib_trans.o $(IB_TRANS_ODIR)/common.o

ifdef GRIDFTP
GFTP_TRANS_DIR ?= $(TRANS_DIR)/gftp_trans
GFTP_TRANS_ODIR = $(GFTP_TRANS_DIR)/obj
GFTP_TRANS_OBJS = $(GFTP_TRANS_ODIR)/gftp_trans.o $(GFTP_TRANS_ODIR)/gftp_drive.o $(GFTP_TRANS_ODIR)/gridftp_api_drive.o $(GFTP_TRANS_ODIR)/io_drive.o
endif # GRIDFTP

TRANS_DIR = $(DSPACESREL_DIR)/trans
TRANS_INC = -I$(TRANS_DIR)/include -I$(IB_TRANS_DIR)/include -I$(GFTP_TRANS_DIR)/include
TRANS_ODIR = $(TRANS_DIR)/obj
TRANS_OBJS = $(TRANS_ODIR)/trans.o $(IB_TRANS_OBJS) $(GFTP_TRANS_OBJS)

PROFILER_DIR = $(DSPACESREL_DIR)/profiler
PROFILER_ODIR = $(PROFILER_DIR)/obj
PROFILER_OBJS = $(PROFILER_ODIR)/profiler.o

DSPACESREL_DIR = $(DSPACESWA_DIR)/dspaces_rel
DSPACESREL_INC = -I$(DSPACESREL_DIR)/include $(SDMCONTROL_INC) $(TRANS_INC) -I$(PROFILER_DIR)/include
DSPACESREL_ODIR = $(DSPACESREL_DIR)/obj
DSPACESREL_OBJS = $(DSPACESREL_ODIR)/remote_interact.o $(DSPACESREL_ODIR)/ds_client.o $(DSPACESREL_ODIR)/ds_drive.o $(SDMCONTROL_OBJS) $(TRANS_OBJS) $(PROFILER_OBJS)

DSPACESWA_INC = -I$(DSPACESWA_DIR)/include $(DSPACESREL_INC)
DSPACESWA_LIB = $(DSPACESWA_DIR)/lib/libdspaces_wa.a
# 
INC = $(DSPACES_INC) $(GLOG_INC) $(BOOST_INC) $(GFTP_INC) $(DSPACESWA_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(MPI_LIB) $(GFTP_LIB)

ODIR = obj
OBJS = $(ODIR)/dataspaces_wa.o $(DSPACESREL_OBJS)
LIBDIR = lib

.PHONY: all lib lclean clean submake_dspaces_rel

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
	ar -cvq $(DSPACESWA_LIB) $(OBJS)

lclean:
	rm -f $(ODIR)/*.o ${APPS}

clean:
	make -C $(DSPACESREL_DIR) clean
	rm -f $(ODIR)/*.o $(LIBDIR)/* ${APPS}
	