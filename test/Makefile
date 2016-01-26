# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install
# DSPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces/dataspaces/install
# CC = /opt/gcc-4.8.2/bin/gcc
# CPP = /opt/gcc-4.8.2/bin/g++
# MPICPP = mpicxx
# GLOG_DIR ?= $(DSPACES_DIR)/../glog-0.3.3/install
# BOOST_DIR ?= /cac/u01/mfa51/Desktop/boost_1_56_0/install
# GFTPINC_DIR ?= 
# GFTPLIB_DIR ?= 
# DSPACESWA_DIR ?= /cac/u01/mfa51/Desktop/dataspaces_wa
# MPI_DIR ?= /cac/u01/mfa51/Desktop/mpich-3.1.2/install
# ##################################################################################################
DSPACES_INC = -I$(DSPACES_DIR)/include
DSPACES_LIB = -L$(DSPACES_DIR)/lib -ldspaces -ldscommon -ldart -lrdmacm

# GLOG_INC = -I$(GLOG_DIR)/include
# GLOG_LIB = -L$(GLOG_DIR)/lib -lglog
# 
ODIR = obj

INC = -I./include $(DSPACES_INC) $(GLOG_INC)
LIB = $(DSPACES_LIB) $(GLOG_LIB)

.PHONY: all clean

APPS := exp

all: ${APPS}

exp: $(ODIR)/exp.o $(ODIR)/ds_test.o $(ODIR)/ds_drive.o $(ODIR)/patch_test.o
	$(MPICPP) $(MPICPP_OPTS) -g -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) $(MPICPP_OPTS) -g -c -o $@ $< $(INC) $(LIB)

clean:
	rm -f $(ODIR)/*.o ${APPS}
