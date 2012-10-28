CC=g++
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g -std=c99
SRV=server
SEL_SRV=server
CLT=client

all: $(SEL_SRV) $(CLT)

$(SEL_SRV):$(SEL_SRV).cpp
	$(CC) -o $(SEL_SRV) $(LIBSOCKET) helpers.h $(SEL_SRV).cpp 

$(CLT):	$(CLT).cpp
	$(CC) 	-o $(CLT) $(LIBSOCKET) helpers.h $(CLT).cpp 

clean:
	rm -f *.o *~
	rm -f $(SEL_SRV) $(CLT)


