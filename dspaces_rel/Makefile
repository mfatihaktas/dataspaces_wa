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
#
INC = $(DATASPACES_INC) $(GLOG_INC) -I/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/include
LIB = $(DATASPACES_LIB) $(GLOG_LIB) $(BOOST_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = gcc
CPP = g++

IDIR = include
ODIR = obj

all: exp dummy_get dummy_put
# dummy2

exp: $(ODIR)/exp.o $(ODIR)/dspaces_drive.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

dummy2: $(ODIR)/dummy2.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

dummy_get: $(ODIR)/dummy_get.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

dummy_put: $(ODIR)/dummy_put.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)

endif

clean:
	rm -f $(ODIR)/*.o