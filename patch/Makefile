BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization -lboost_iostreams -lboost_filesystem

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

PATCH_DIR ?= $(DSPACESWA_DIR)/patch
PATCH_INC = -I$(PATCH_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) -I$(DSPACES_WA_DIR)/plot/include -I$(DSPACES_WA_DIR)/dspaces_rel/profiler/include $(PREFETCH_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}