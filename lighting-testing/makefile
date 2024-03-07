#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
BSO     = /home/kknopp/Documents/BSO
FADE    = /home/kknopp/Documents/include_fade2d
FADELIB = /usr/lib

ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO) -I$(FADE) -L$(FADELIB)

#compiler settings
CPP   = ccache g++ -std=c++20
FLAGS  = -O3 -march=native -lglut -lGL -lGLU -lpthread -lfade2d

#test suites
MAINFILE = main.cpp

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) -o get_results $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f get_results
