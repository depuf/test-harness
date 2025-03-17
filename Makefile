CFLAGS=-W -Wall -g -I/usr/local/include
LDFLAGS=-g -L/usr/local/lib
PROGS=thv1 thv2 thv3
LIBRARIES=

all: $(PROGS)

thv1: thv1.o p1fxns.o
	gcc $(LDFLAGS) -o $@ $^

thv2: thv2.o p1fxns.o
	gcc $(LDFLAGS) -o $@ $^

thv3: thv3.o p1fxns.o cirque.o$(LIBRARIES)
	gcc $(LDFLAGS) -o $@ $^

thv4: thv4.o p1fxns.o $(LIBRARIES)
	gcc $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(PROGS)

thv1.o: thv1.c p1fxns.h
thv2.o: thv2.c p1fxns.h
thv3.o: thv3.c p1fxns.h
p1fxns.o: p1fxns.c p1fxns.h
cirque.o: cirque.c cirque.h