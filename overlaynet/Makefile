DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

#
INC = $(BOOSTASIO_INC) $(GLOG_INC) -I/cac/u01/mfa51/Desktop/dataspaces_wa/overlaynet/include
LIB = $(BOOSTASIO_LIB) $(BOOST_LIB) $(GLOG_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CPP = g++

#BGLFLAGS = 
#LIB = -lpthread

IDIR = include
ODIR = obj

#exp_put exp_get
all: exp exp2
#exp2

exp2: $(ODIR)/exp2.o $(ODIR)/dht_client.o $(ODIR)/packet.o
	$(CPP) -o $@ $^ $(INC) $(LIB)

exp: $(ODIR)/exp.o $(ODIR)/dht_server.o $(ODIR)/packet.o $(ODIR)/dht_node.o
	$(CPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(INC) $(LIB)

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

clean:
	rm -f $(ODIR)/*.o
