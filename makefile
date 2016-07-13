CFLAGS += -g -Wall
LDFLAGS += -lgdbm

dbm: dbm.o

clean:
	rm -f *.o dbm

.PHONY: clean
