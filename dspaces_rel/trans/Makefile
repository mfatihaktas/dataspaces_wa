# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lboost_thread -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

IBVERBS_LIB = -L/usr/lib64 -libverbs -lrdmacm
# IB_LIB = -lrdmacm -libverbs -lpthread 
# 
PATCH_INC = -I$(DSPACESWA_DIR)/patch/include

ifdef GRIDFTP
GFTP_INC = -I$(GFTPINC_DIR) #-I/usr/lib64/globus/include
GFTP_LIB = -L$(GFTPLIB_DIR) -lglobus_ftp_client
endif # GRIDFTP
IB_TRANS_DIR ?= $(TRANS_DIR)/ib_trans
IB_TRANS_ODIR = $(IB_TRANS_DIR)/obj
# IB_TRANS_OBJS = $(IB_TRANS_ODIR)/ib_trans.o $(IB_TRANS_ODIR)/common.o
IB_TRANS_OBJS = $(IB_TRANS_ODIR)/ib_trans.o $(IB_TRANS_ODIR)/ib_server.o $(IB_TRANS_ODIR)/ib_client.o $(IB_TRANS_ODIR)/ib_conn.o

TCP_TRANS_DIR ?= $(TRANS_DIR)/tcp_trans
TCP_TRANS_ODIR = $(TCP_TRANS_DIR)/obj
TCP_TRANS_OBJS = $(TCP_TRANS_ODIR)/tcp_trans.o $(TCP_TRANS_ODIR)/tcp_server.o $(TCP_TRANS_ODIR)/tcp_client.o $(TCP_TRANS_ODIR)/patch_tcp.o

ifdef GRIDFTP
GFTP_TRANS_DIR ?= $(TRANS_DIR)/gftp_trans
GFTP_TRANS_ODIR = $(GFTP_TRANS_DIR)/obj
GFTP_TRANS_OBJS = $(GFTP_TRANS_ODIR)/gftp_trans.o $(GFTP_TRANS_ODIR)/gftp_drive.o $(GFTP_TRANS_ODIR)/gridftp_api_drive.o $(GFTP_TRANS_ODIR)/io_drive.o
endif # GRIDFTP

TRANS_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/trans
TRANS_INC = $(PATCH_INC) -I$(IB_TRANS_DIR)/include -I$(TCP_TRANS_DIR)/include -I$(GFTP_TRANS_DIR)/include -I$(TRANS_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) $(GFTP_INC) $(TRANS_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) $(GFTP_LIB)

ODIR = obj
OBJS = $(ODIR)/exp.o $(ODIR)/trans.o $(IB_TRANS_OBJS) $(TCP_TRANS_OBJS) $(GFTP_TRANS_OBJS)

.PHONY: all submake_ib_trans submake_tcp_trans submake_gftp_trans lclean clean

APPS := exp

all: submake_ib_trans submake_tcp_trans submake_gftp_trans ${APPS}

submake_ib_trans:
	make -C $(IB_TRANS_DIR)

submake_tcp_trans:
	make -C $(TCP_TRANS_DIR)
	
submake_gftp_trans:
ifdef GRIDFTP
	make -C $(GFTP_TRANS_DIR)
endif # GRIDFTP

exp: $(OBJS)
	${CPP} $(CPP_OPTS) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) $(CPP_OPTS) -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS}

clean:
	make -C $(IB_TRANS_DIR) clean
	make -C $(TCP_TRANS_DIR) clean
ifdef GRIDFTP
	make -C $(GFTP_TRANS_DIR) clean
endif # GRIDFTP
	rm -f $(ODIR)/*.o ${APPS}
	