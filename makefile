CFLAGS += -g -Wall
LDLIBS += -L/usr/lib/x86_64-linux-gnu/ -lgdbm

dbm: dbm.o

test: dbm
	@echo 'TESTING basicTestSuite.yaml'
	@python2 test.py basicTestSuite.yaml

clean:
	rm -f *.o dbm

.PHONY: clean test
