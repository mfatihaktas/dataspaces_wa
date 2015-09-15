# CPP = g++
# DSPACES_WA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# 
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization -lboost_iostreams -lboost_filesystem

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog
# 
IDIR = include
ODIR = obj

INC = -I$(DSPACES_WA_DIR)/plot/include -I$(DSPACES_WA_DIR)/dspaces_rel/profiler/include -I$(IDIR) $(GLOG_INC) $(BOOST_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/sim.o $(ODIR)/markov.o $(ODIR)/palgorithm.o 
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}