# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

IBTRANS_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/ib_trans
IBTRANS_INC = -I$(IBTRANS_DIR)/include

IBVERBS_LIB = -L/usr/lib64 -libverbs -lrdmacm

# IB_LIB = -lrdmacm -libverbs -lpthread 
#
INC = $(GLOG_INC) $(BOOST_INC) $(IBTRANS_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) 

IDIR = include
ODIR = obj

.PHONY: lclean clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/common.o $(ODIR)/ib_delivery.o
	${CPP} $(CPP_OPTS)  -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) $(CPP_OPTS) -c -o $@ $< $(INC) $(LIB)

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	rm -f obj/*.o ${APPS}