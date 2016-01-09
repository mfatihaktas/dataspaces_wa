# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

IBVERBS_LIB = -L/usr/lib64 -libverbs -lrdmacm
# IB_LIB = -lrdmacm -libverbs -lpthread

# IB_TRANS_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/trans/ib_trans
# 
INC = $(GLOG_INC) $(BOOST_INC) -I./include
LIB = $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) 

ODIR = obj
OBJS = $(ODIR)/exp.o $(ODIR)/ib_trans.o $(ODIR)/ib_server.o $(ODIR)/ib_client.o $(ODIR)/ib_conn.o

.PHONY: all lclean clean

APPS := exp

all: ${APPS}

exp: $(OBJS)
	${CPP} $(CPP_OPTS) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) $(CPP_OPTS) -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	rm -f obj/*.o ${APPS}