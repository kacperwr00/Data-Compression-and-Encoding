COMPILER = g++

COMPILER_SETTINGS = -std=c++2a 

VERBOSE_COMPILER_SETTINGS = -Wall -Wextra -pedantic -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -O3 -std=c++2a

.PHONY = all debug verbose clean

all: 
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o dekoder dekoder.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o koder koder.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o sprawdz sprawdz.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o szum szum.cpp $(SHARED_LIBS)

debug:
	$(COMPILER) $(COMPILER_SETTINGS) -g -o dekoder dekoder.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o koder koder.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o sprawdz sprawdz.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o szum szum.cpp $(SHARED_LIBS)

verbose:
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o dekoder dekoder.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o koder koder.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o sprawdz sprawdz.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o szum szum.cpp $(SHARED_LIBS)

clean:
	rm -f dekoder koder sprawdz szum