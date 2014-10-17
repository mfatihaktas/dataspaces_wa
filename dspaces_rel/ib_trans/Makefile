BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
BOOST_INC = -I$(BOOST_DIR)/include
BOOST_LIB = -L$(BOOST_DIR)/lib -lboost_system -lboost_thread -lboost_serialization
# BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

GLOG_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install
GLOG_INC = -I$(GLOG_DIR)/include
GLOG_LIB = -L$(GLOG_DIR)/lib -l:libglog.a

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so

IBTRANS_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/ib_trans
IBTRANS_INC = -I$(IBTRANS_DIR)/include

IBVERBS_LIB = -L/usr/lib64 -libverbs -lrdmacm

# IB_LIB = -lrdmacm -libverbs -lpthread 
#
INC = $(GLOG_INC) $(BOOST_INC) $(IBTRANS_INC)
LIB = $(GLOG_LIB) $(BOOST_LIB) $(IBVERBS_LIB) 
# $(IB_LIB)

MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
CC = /opt/gcc-4.8.2/bin/gcc
CPP = /opt/gcc-4.8.2/bin/g++
CPP_OPTS = #-pthread

IDIR = include
ODIR = obj

.PHONY: lclean clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/common.o $(ODIR)/ib_delivery.o
	${CPP} $(CPP_OPTS)  -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) $(CPP_OPTS) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
client: $(ODIR)/client.o $(ODIR)/common.o
	${CC} -o $@ $^ $(INC) $(LIB)

server: $(ODIR)/server.o $(ODIR)/common.o
	${CC} -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

lclean:
	rm -f $(ODIR)/*.o ${APPS} 

clean:
	rm -f obj/*.o ${APPS}