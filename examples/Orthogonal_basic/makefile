# Specify location of libraries
BOOST 	= /usr/include/boost
EIGEN 	= /usr/include/eigen
BSO   	= /home/tessa/BSO-toolbox-V2
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# Choose the Delaunay triangulation libary used: set DELAUNAY=cgal (default) or DELAUNAY=fade
DELAUNAY ?= fade
FADE_ROOT ?= /home/tessa/Libaries/fadeRelease_v1.93/fadeRelease_v1.93
ifeq ($(DELAUNAY),fade)
	DELAUNAY_FLAGS = -lfade2d
	DELAUNAY_DEFS = -DBSO_USE_FADE
	D_LIB = $(DELAUNAY_DEFS) -I${FADE_ROOT}/include_fade2d
else ifeq ($(DELAUNAY),cgal)
	DELAUNAY_FLAGS = -lgmp
	DELAUNAY_DEFS = -DBSO_USE_CGAL
	D_LIB = $(DELAUNAY_DEFS)
else
	$(error Unknown DELAUNAY "$(DELAUNAY)". Use cgal or fade.)
endif

# Compiler settings
CPP = g++ -g -std=c++17
FLAGS = -march=native -lglut -lGL -lGLU -lpthread $(DELAUNAY_FLAGS)

# Specify file(s) to be compiled
MAINFILE = main.cpp

# Specify name of executable
EXE = main

.PHONY: all cls clean

# Make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -o main $(ALL_LIB) $(MAINFILE) $(FLAGS)

# Remove previously compiled executable
clean:
	@rm -f $(EXE)