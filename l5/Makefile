COMPILER = g++

COMPILER_SETTINGS =  -std=c++2a -fopenmp

VERBOSE_COMPILER_SETTINGS = -Wall -pedantic -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -O3 -std=c++2a

.PHONY = all debug verbose clean

all: 
	$(COMPILER) $(COMPILER_SETTINGS) -o quantization quantization.cpp $(SHARED_LIBS)

debug:
	$(COMPILER) $(COMPILER_SETTINGS) -g -o quantization quantization.cpp $(SHARED_LIBS)

verbose:
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o quantization quantization.cpp $(SHARED_LIBS)

clean:
	rm -f quantization