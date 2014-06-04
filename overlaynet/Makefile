DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64 -lboost_system -lpthread -lboost_thread-mt

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CPP = g++

#BGLFLAGS = 
#LIB = -lpthread

SFS = $(exp_sf)
OBJ = $(exp_of)

#%.o: %.c $(SFS)
#	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)
#exp_put exp_get
all: chat_server chat_client

chat_server: chat_server.o
	$(CPP) -o $@ $^ $(BOOSTASIO_INC) $(BOOSTASIO_LIB)

chat_client: chat_client.o
	$(CPP) -o $@ $^ $(BOOSTASIO_INC) $(BOOSTASIO_LIB)

chat_server.o: chat_server.cpp
	$(CPP) -c -o $@ $< $(BOOSTASIO_INC) $(BOOSTASIO_LIB)

#%.o: %.c
#	$(CPP) -c -o $@ $< $(BOOSTASIO_INC)
#
exp_put: exp_put.o common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

exp_get: exp_get.o common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

exp: exp.o common.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

exp_put.o: exp_put.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

exp_get.o: exp_get.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

exp.o: exp.cpp
	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

common.o: common.c
	$(MPICC) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

clean:
	rm -f *.o
