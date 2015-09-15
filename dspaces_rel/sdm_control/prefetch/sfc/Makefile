# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# ##################################################################################################
BOOSTASIO_LIB = -L$(BOOSTASIO_DIR)

BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization -lboost_iostreams -lboost_filesystem
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_INC = -I$(GLOG_DIR)/include
# GLOG_LIB = -L$(GLOG_DIR)/lib -l:libglog.a
GLOG_LIB = -L$(GLOG_DIR)/lib -lglog

SFC_DIR ?= $(DSPACESWA_DIR)/dspaces_rel/sdm_control/sfc
SFC_INC = -I$(SFC_DIR)/include
#
INC = $(GLOG_INC) $(BOOST_INC) $(SFC_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB)

ODIR = obj

.PHONY: clean all

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/sim.o $(ODIR)/sfc.o $(ODIR)/hilbert.o
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

$(ODIR)/%.o: %.c
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}