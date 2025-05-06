# specify location of libraries
BOOST 	= /usr/include/boost
EIGEN 	= /usr/include/eigen
BSO   	= /home/tessa/BSO-toolbox-V2
FADE	= $(BSO)/code_for_colaboration_Leiden/libraries/fadeRelease_v2.13
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# compiler settings
CPP = g++ -std=c++20
FLAGS = -O3 -march=native -lglut -lGL -lGLU -lpthread -lfade2d_v2.13 -Wl,-rpath=${FADE}/lib_ubuntu22.04_x86_64
D_LIB  = -I${FADE}/include_fade2d -L${FADE}/lib_ubuntu22.04_x86_64

# specify file(s) to be compiled
MAINFILE = main.cpp

# specify name of executable
EXE = get_results

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -L$(BSO)/libraries -o $(EXE) $(ALL_LIB) $(MAINFILE) $(FLAGS)

# remove previously compiled executable
clean:
	@rm -f $(EXE)
