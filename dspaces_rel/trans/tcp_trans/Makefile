# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

TCP_TRANS_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/trans/tcp_trans
# 
INC = $(GLOG_INC) $(BOOST_INC) -I$(TCP_TRANS_DIR)/include
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj
OBJS = $(ODIR)/tcp_trans.o $(ODIR)/tcp_server.o $(ODIR)/tcp_client.o

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(OBJS)
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}
