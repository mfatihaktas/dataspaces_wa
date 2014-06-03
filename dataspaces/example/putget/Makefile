DATASPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0
DATASPACES_INC=-I/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/include
DATASPACES_LIB=-L/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/lib -ldspaces -ldscommon -ldart -lrdmacm
CC=mpicc

DEPS=test_put.c test_get.c common.c
OBJ=testw.o testr.o common.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(DATASPACES_INC) $(DATASPACES_LIB)

all: test_put test_get

test_put: test_put.o common.o
	$(CC) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

test_get: test_get.o common.o
	$(CC) -o $@ $^ $(DATASPACES_INC) $(DATASPACES_LIB)

.PHONY: clean

clean:
	rm -f *.o test_put test_get
