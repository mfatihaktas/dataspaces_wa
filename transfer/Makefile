DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

GFTP_INC = -I/usr/include/globus -I/usr/lib64/globus/include
GFTP_LIB = -L/usr/lib64/ -l:libglobus_ftp_client.so
#
#INC = $(BOOSTASIO_INC) $(GLOG_INC) -I/cac/u01/mfa51/Desktop/dataspaces_wa/overlaynet/include
#LIB = $(BOOSTASIO_LIB) $(BOOST_LIB) $(GLOG_LIB)

INC = $(GFTP_INC) $(GLOG_INC)
LIB = $(GFTP_LIB) $(GLOG_LIB) $(BOOST_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = gcc
CPP = g++

IDIR = include
ODIR = obj

#exp_put exp_get
all: exp

ctos: $(ODIR)/client_to_server.o
	$(CPP) -o $@ $^ $(INC) $(LIB)

exp: $(ODIR)/exp.o
	$(CPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
endif

clean:
	rm -f $(ODIR)/*.o