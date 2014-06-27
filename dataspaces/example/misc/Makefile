DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm

BOOST_LIB = -lboost_system -lpthread -lboost_thread-mt -lboost_serialization

BOOSTASIO_INC = #-I/opt/boost_1_55_0
BOOSTASIO_LIB = -L/opt/matlab/R2013a/bin/glnxa64

GLOG_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/include
GLOG_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/glog-0.3.3/install/lib -l:libglog.a

#
#INC = $(BOOSTASIO_INC) $(GLOG_INC) $(DATASPACES_INC) -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/example/misc/include
#LIB = $(BOOSTASIO_LIB) $(BOOST_LIB) $(GLOG_LIB) $(DATASPACES_LIB)

INC = $(GLOG_INC) $(DATASPACES_INC)
LIB = $(GLOG_LIB) $(DATASPACES_LIB)

MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 
CPP = g++

IDIR = include
ODIR = obj

all: misc_put misc_get

misc_put: $(ODIR)/misc_put.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(INC) $(LIB)

misc_get: $(ODIR)/misc_get.o
	$(MPICPP) $(MPICPPOPTS) -o $@ $^ $(INC) $(LIB)

$(ODIR)/%.o: %.cpp
	$(MPICPP) -c -o $@ $< $(INC) $(LIB)

ifeq ("x","y")
endif

clean:
	rm -f $(ODIR)/*.o
