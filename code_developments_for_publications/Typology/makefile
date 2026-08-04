# specify location of libraries
BOOST = /usr/include/boost
EIGEN = /usr/include/eigen
FADE	= /home/tessa/Libaries/fadeRelease_v1.93/fadeRelease_v1.93
BSO 	= /home/tessa/BSO-toolbox-V2
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

#compiler settings
CPP   = g++ -g -std=c++17 -fmax-errors=5
FLAGS  = -O -march=native -lglut -lGL -lGLU -lpthread $(DELAUNAY_FLAGS)

# specify file(s) to be compiled
MAINFILE = main.cpp

# specify name of executable
EXE = topopt_ele_wise

.PHONY: all clean

# definition of arguments for make command
# argument to call compiler and compile executable called "design_response_grammar"
all:
	$(CPP) $(D_LIB) -o topopt_ele_wise $(ALL_LIB) $(MAINFILE) $(FLAGS)
cls:
	clear
	clear
# remove previously compiled executable
clean:
	@rm -f $(EXE)
