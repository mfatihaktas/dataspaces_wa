BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization -lboost_iostreams -lboost_filesystem
# BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

PATCH_INC = -I$(DSPACESWA_DIR)/patch/include

RESILIENCE_DIR ?= $(DSPACESWA_DIR)/resilience
RESILIENCE_INC = $(PATCH_INC) -I$(RESILIENCE_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) $(RESILIENCE_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj
OBJS = $(ODIR)/exp.o $(ODIR)/xor.o

.PHONY: all eclean clean

APPS := exp

all: ${APPS}

exp: $(OBJS)
	$(MPICPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -g -c -o $@ $< $(INC) $(LIB)

eclean:
	rm -f $(ODIR)/exp.o ${APPS}

clean:
	rm -f $(ODIR)/*.o ${APPS}