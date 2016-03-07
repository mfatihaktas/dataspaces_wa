BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lboost_thread -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

IBVERBS_LIB = -L/usr/lib64 -libverbs -lrdmacm
# IB_LIB = -lrdmacm -libverbs -lpthread
# 
PATCH_INC = -I$(DSPACESWA_DIR)/patch/include

IB_TRANS_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/trans/ib_trans
IB_TRANS_INC = $(PATCH_INC) -I$(IB_TRANS_DIR)/include
# 
INC = $(GLOG_INC) $(BOOST_INC) $(IB_TRANS_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB)

ODIR = obj
# $(ODIR)/ib_server.o $(ODIR)/ib_client.o $(ODIR)/ib_conn.o
OBJS = $(ODIR)/exp.o $(ODIR)/ib_trans.o $(ODIR)/common.o

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