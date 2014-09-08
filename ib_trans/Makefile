DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0
DATASPACES_INC = -I$(DATASPACES_DIR)/install/include
DATASPACES_LIB = -L$(DATASPACES_DIR)/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so

SUB_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/ib_trans/include

IB_LIB = -lrdmacm -libverbs -lpthread
#
INC = $(SUB_INC) $(GLOG_INC) #$(DATASPACES_INC)
LIB = $(IB_LIB) $(GLOG_LIB) $(BOOST_LIB) #$(DATASPACES_LIB) 

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = gcc
CPP = g++

IDIR = include
ODIR = obj

.PHONY: clean

#CFLAGS	:= -Wall -Werror -g
#LD			:= gcc
#LDFLAGS := ${LDFLAGS} -lrdmacm -libverbs -lpthread

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/common.o $(ODIR)/ib_delivery.o
	${CPP} -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
client: $(ODIR)/client.o $(ODIR)/common.o
	${CC} -o $@ $^ $(INC) $(LIB)

server: $(ODIR)/server.o $(ODIR)/common.o
	${CC} -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

clean:
	rm -f obj/*.o ${APPS}