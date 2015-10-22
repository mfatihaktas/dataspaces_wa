# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
# BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)
# -lboost_iostreams -lboost_filesystem
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
# GLOG_LIB = -L$(GLOG_DIR)/lib -l:libglog.a
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

PREFETCH_DIR ?= $(SDM_CONTROL_DIR)/prefetch
PREFETCH_ODIR = $(PREFETCH_DIR)/obj
PREFETCH_OBJS = $(PREFETCH_ODIR)/prefetch.o $(PREFETCH_ODIR)/markov.o $(PREFETCH_ODIR)/sfc.o $(PREFETCH_ODIR)/hilbert.o

SDM_CONTROL_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/sdm_control
SDM_CONTROL_INC = -I$(SDM_CONTROL_DIR)/include -I$(PREFETCH_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) $(SDM_CONTROL_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj
OBJS = $(ODIR)/exp.o $(ODIR)/sdm_control.o $(ODIR)/sdm_node.o $(ODIR)/sdm_server.o $(ODIR)/sdm_client.o $(ODIR)/packet.o $(ODIR)/patch_sdm.o $(PREFETCH_OBJS)

.PHONY: all submake_prefetch lclean clean

APPS := exp

all: submake_prefetch ${APPS}

submake_prefetch:
	make -C $(PREFETCH_DIR)

exp: $(OBJS)
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS}

clean:
	make -C $(PREFETCH_DIR) clean
	rm -f $(ODIR)/*.o ${APPS}