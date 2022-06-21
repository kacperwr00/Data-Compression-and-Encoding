COMPILER = g++

COMPILER_SETTINGS =  -std=c++2a

VERBOSE_COMPILER_SETTINGS = -Wall -pedantic -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -O3 -std=c++17

.PHONY = all debug verbose clean

all: 
	$(COMPILER) $(COMPILER_SETTINGS) -o lzwDecode lzwDecode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -o lzwEncode lzwEncode.cpp $(SHARED_LIBS)

debug:
	$(COMPILER) $(COMPILER_SETTINGS) -g -o lzwDecode lzwDecode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o lzwEncode lzwEncode.cpp $(SHARED_LIBS)

verbose:
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o lzwDecode lzwDecode.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o lzwEncode lzwEncode.cpp $(SHARED_LIBS)

clean:
	rm -f lzwDecode lzwEncode