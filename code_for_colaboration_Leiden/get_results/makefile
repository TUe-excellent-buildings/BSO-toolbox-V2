#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
FADE	= /home/tessa/Libaries/fadeRelease_v1.93/fadeRelease_v1.93
BSO     = /home/tessa/toolbox_current
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

#compiler settings
CPP   = g++ -std=c++14
FLAGS  = -O3 -march=native -lglut -lGL -lGLU -lpthread -lfade2d
D_LIB  = -I${FADE}/include_fade2d

#test suites
MAINFILE = main.cpp

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -o get_results $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f get_results