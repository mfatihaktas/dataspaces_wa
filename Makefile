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

SUB_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/include -I/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/include
#
INC = $(DATASPACES_INC) $(GLOG_INC) $(SUB_INC)
LIB = $(DATASPACES_LIB) $(GLOG_LIB) $(BOOST_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CC = gcc
CPP = g++

IDIR = include
ODIR = obj

DSPACES_REL_DIR = dspaces_rel
DSPACES_REL_IDIR = $(DSPACES_REL_DIR)/include
DSPACES_REL_ODIR = $(DSPACES_REL_DIR)/obj

.PHONY: all clean submake_dspaces_rel dataspaces_wa

all: submake_dspaces_rel dataspaces_wa

submake_dspaces_rel:
	make -C dspaces_rel

dataspaces_wa: $(ODIR)/dataspaces_wa.o $(DSPACES_REL_ODIR)/ds_client.o $(DSPACES_REL_ODIR)/ds_drive.o
	$(MPICPP) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(INC) $(LIB)
endif

clean:
	rm -f $(ODIR)/*.o $(DSPACES_REL_ODIR)/*.o