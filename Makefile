PRG=gnu.exe
GCC=g++
GCCFLAGS=-O3 -Wall -Wextra -std=c++11 -latomic -mcx16

VALGRIND_OPTIONS=-q --leak-check=full
DIFFLAGS=--strip-trailing-cr -y --suppress-common-lines 

OBJECTS0=
DRIVER0=driver.cpp

OSTYPE := $(shell uname)
ifeq ($(OSTYPE),Linux)
CYGWIN=
else
CYGWIN=-Wl,--enable-auto-import
endif

gcc0:
	$(GCC) -o $(PRG) $(CYGWIN) $(DRIVER0) $(OBJECTS0) $(GCCFLAGS) -pthread
0 1 2 3:
	@echo "running test$@"
	@echo "should run in less than 1000 ms"
	./$(PRG) $@ >studentout$@
	@echo "lines after the next are mismatches with master output -- see out$@"
	diff out$@ studentout$@ $(DIFFLAGS)
mem1:
	@echo "should run in less than 4000 ms"
	valgrind $(VALGRIND_OPTIONS) ./$(PRG) $(subst mem,,$@) 1>/dev/null 2>difference$@
	@echo "lines after this are memory errors"; cat difference$@
clean:
	rm -f *.exe *.o *.obj
