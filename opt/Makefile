CFLAGS=-I.. -I. -O3 -Wall -lgc

test-kenneth: CFLAGS += -DTEST_KENNETH
test-kenneth: kenneth
	./kenneth

kenneth: kenneth.c ../fond/linked-list.c ../fond/symbol.c ../fond/string.o

