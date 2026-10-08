# specify location of libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
FADE	= /usr/include/fadeRelease_v1.93
BSO     = /usr/include/BSO-toolbox-V2
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

#compiler settings
CPP   = g++ -std=c++14
FLAGS  = -O3 -march=native -lglut -lGL -lGLU -lpthread -lfade2d
D_LIB  = -I${FADE}/include_fade2d

# specify file(s) to be compiled
MAINFILE = main.cpp

.PHONY: all cls clean

# Definition of arguments for make command
# Argument to call compiler and compile executable called "main"
# Command to use: 'make cls clean all'
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -o main $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f main