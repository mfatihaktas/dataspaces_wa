DATASPACES_DIR = /cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC = -I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB = -L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm
MPICC = mpicc
MPICPP = mpic++
MPICPPOPTS = 

#BGLFLAGS = 
#LIB = -lpthread

SFS = $(exp_sf)
OBJ = $(exp_of)

#%.o: %.c $(SFS)
#	$(MPICPP) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)
all: exp_put exp_get

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
