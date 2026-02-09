#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
BSO     = /home/tessa/Sanne_BSO_toolbox/MSc_2022_Van_der_Wal/toolbox
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

#compiler settings
CPP   = g++ -std=c++14
FLAGS  = -O3 -march=native -lglut -lGL -lGLU -lpthread

#test suites
MAINFILE = main.cpp

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) -o main $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f main
