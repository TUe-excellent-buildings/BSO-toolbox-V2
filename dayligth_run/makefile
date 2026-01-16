#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
FADE  	= /home/tessa/Libaries/fadeRelease_v1.93/fadeRelease_v1.93
BSO   	= /home/tessa/MSc_2025_Naus
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

#compiler settings
CPP = g++ -std=c++17# -g
FLAGS = -O -march=native -lglut -lGL -lGLU -lpthread -lfade2d
D_LIB  = -I${FADE}/include_fade2d

#test suites
MAINFILE = DL_Loop.cpp#scdp_performance_clusters.cpp

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -o main $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f main