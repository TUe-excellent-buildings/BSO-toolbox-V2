# specify location of libraries
BOOST 	= /usr/include/boost
EIGEN 	= /usr/include/eigen
BSO   	= /home/tessa/BSO-toolbox-V2
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# set DELAUNAY=cgal (default) or DELAUNAY=fade when invoking make to choose the Delaunay triangulation backend
DELAUNAY ?= fade
FADE_ROOT ?= $(BSO)/code_for_colaboration_Leiden/libraries/fadeRelease_v2.13
ifeq ($(DELAUNAY),fade)
	DELAUNAY_FLAGS = -lfade2d_v2.13 -Wl,-rpath=${FADE_ROOT}/lib_ubuntu22.04_x86_64
	DELAUNAY_DEFS = -DBSO_USE_FADE
	D_LIB = $(DELAUNAY_DEFS) -I${FADE_ROOT}/include_fade2d -L${FADE_ROOT}/lib_ubuntu22.04_x86_64
else ifeq ($(DELAUNAY),cgal)
	DELAUNAY_FLAGS = -lgmp
	DELAUNAY_DEFS = -DBSO_USE_CGAL
	D_LIB = $(DELAUNAY_DEFS)
else
	$(error Unknown DELAUNAY "$(DELAUNAY)". Use cgal or fade.)
endif

# compiler settings
CPP = g++ -std=c++20
FLAGS = -O3 -march=native -lglut -lGL -lGLU -lpthread $(DELAUNAY_FLAGS)

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
