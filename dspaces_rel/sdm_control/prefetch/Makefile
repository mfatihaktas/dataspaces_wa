# CPP = g++
# DSPACES_WA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# -lboost_serialization -lboost_iostreams -lboost_filesystem
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

PREFETCH_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/sdm_control/prefetch
PREFETCH_INC = -I$(PREFETCH_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) -I$(DSPACES_WA_DIR)/plot/include -I$(DSPACES_WA_DIR)/dspaces_rel/profiler/include $(PREFETCH_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/sim.o $(ODIR)/prefetch.o $(ODIR)/markov.o $(ODIR)/sfc.o $(ODIR)/hilbert.o
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

$(ODIR)/%.o: %.c
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}