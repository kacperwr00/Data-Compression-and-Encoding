COMPILER = g++

COMPILER_SETTINGS = -std=c++2a  -fopenmp

VERBOSE_COMPILER_SETTINGS = -Wall -Wextra -pedantic -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -O3 -std=c++2a -fopenmp

.PHONY = all debug verbose clean

all: 
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o encode encode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o encodeRownomiernie encodeRownomiernie.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o decode decode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -O3 -o mse mse.cpp $(SHARED_LIBS)

debug:
	$(COMPILER) $(COMPILER_SETTINGS) -g -o encode encode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o encodeRownomiernie encodeRownomiernie.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o decode decode.cpp $(SHARED_LIBS)
	$(COMPILER) $(COMPILER_SETTINGS) -g -o mse mse.cpp $(SHARED_LIBS)

verbose:
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o encode encode.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o encodeRownomiernie encodeRownomiernie.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o decode decode.cpp $(SHARED_LIBS)
	$(COMPILER) $(VERBOSE_COMPILER_SETTINGS) -o mse mse.cpp $(SHARED_LIBS)

clean:
	rm -f decode encode mse encodeRownomiernie