# specify location of libraries
BOOST 	= /usr/include/boost
EIGEN 	= /usr/include/eigen
FADE	= /home/tessa/Libaries/fadeRelease_v1.93/fadeRelease_v1.93															  
BSO   	= /home/tessa/BSO-toolbox-V2
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# compiler settings
CPP = g++ -std=c++17
FLAGS = -O3 -march=native -lglut -lGL -lGLU -lpthread -lfade2d
D_LIB  = -I${FADE}/include_fade2d								 

# specify file(s) to be compiled
MAINFILE = main.cpp

# specify name of executable
EXE = example

.PHONY: all clean

# definition of arguments for make command
# argument to call compiler and compile executable called "example"
all:
	$(CPP) $(D_LIB) -o example $(ALL_LIB) $(MAINFILE) $(FLAGS)

# remove previously compiled executable
clean:
	@rm -f $(EXE)
