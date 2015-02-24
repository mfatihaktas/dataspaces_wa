DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
MPICPP = /cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
MPICPP_OPTS = # -pthread
# GLOG_DIR ?= /cac/u01/mfa51/Desktop/glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# GFTPINC_DIR ?= 
# GFTPLIB_DIR ?= 
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# MPI_DIR ?= /cac/u01/mfa51/Desktop/mpich-3.1.2/install
# ##################################################################################################
DSPACES_INC = -I$(DSPACES_DIR)/include
DSPACES_LIB = -L$(DSPACES_DIR)/lib -ldspaces -ldscommon -ldart -lrdmacm

IDIR = include
ODIR = obj

INC = -I$(IDIR) $(DSPACES_INC)
LIB = $(DSPACES_LIB)

.PHONY: all clean

APPS := exp

all: ${APPS}
exp: $(ODIR)/exp.o $(ODIR)/ds_test.o $(ODIR)/ds_drive.o
	$(MPICPP) $(MPICPP_OPTS) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) $(MPICPP_OPTS) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}