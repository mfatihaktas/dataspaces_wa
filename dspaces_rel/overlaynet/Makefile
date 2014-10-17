BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lpthread -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install
GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -l:libglog.a

OVERLAYNET_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/overlaynet
OVERLAYNET_INC = -I$(OVERLAYNET_DIR)/include
#
INC = $(BOOSTASIO_INC) $(GLOG_INC) $(BOOST_INC) $(OVERLAYNET_INC)
LIB = $(BOOSTASIO_LIB) $(GLOG_LIB) $(BOOST_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = /opt/gcc-4.8.2/bin/gcc
CPP = /opt/gcc-4.8.2/bin/g++

IDIR = include
ODIR = obj

#exp_put exp_get

.PHONY: lclean clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/dht_node.o $(ODIR)/dht_server.o $(ODIR)/dht_client.o $(ODIR)/packet.o
	$(CPP) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -g -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
#boost chat example
#all: chat_server chat_client
chat_server: $(ODIR)/chat_server.o
	$(CPP) -o $@ $^ $(BOOSTASIO_INC) $(BOOSTASIO_LIB)

chat_client: $(ODIR)/chat_client.o
	$(CPP) -o $@ $^ $(BOOSTASIO_INC) $(BOOSTASIO_LIB)

###
$(ODIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(BOOSTASIO_INC) $(BOOSTASIO_LIB)
#dataspaces putget example
exp_put: $(ODIR)/exp_put.o $(ODIR)/common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

exp_get: $(ODIR)/exp_get.o $(ODIR)/common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

exp: $(ODIR)/exp.o $(ODIR)/common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

$(ODIR)/exp_put.o: exp_put.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

$(ODIR)/exp_get.o: exp_get.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

$(ODIR)/exp.o: exp.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

$(ODIR)/common.o: common.c
	$(MPICC) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

#%.o: %.c $(SFS)
#	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)
endif

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	rm -f $(ODIR)/*.o ${APPS}
