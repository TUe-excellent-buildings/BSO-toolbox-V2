# # Specify location of libraries
# BOOST 	= C:/Program Files/CodeBlocks/boost_1_81_0
# EIGEN 	= C:/Program Files/CodeBlocks/eigen-3.4.0
# CGAL 	= C:/Program Files/CodeBlocks/CGAL-6.1
# BSO   	= ${workspaceFolder}
# ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# # Choose the Delaunay triangulation libary used: set DELAUNAY=cgal (default) or DELAUNAY=fade
# DELAUNAY ?= cgal
# FADE_ROOT ?= C:\Program Files\CodeBlocks\Fade2D
# ifeq ($(DELAUNAY),fade)
# 	DELAUNAY_FLAGS = -lfade2d
# 	DELAUNAY_DEFS = -DBSO_USE_FADE
# 	D_LIB = $(DELAUNAY_DEFS) -I${FADE_ROOT}/include
# else ifeq ($(DELAUNAY),cgal)
# 	DELAUNAY_FLAGS = -lgmp
# 	DELAUNAY_DEFS = -DBSO_USE_CGAL
# 	D_LIB = $(DELAUNAY_DEFS)
# else
# 	$(error Unknown DELAUNAY "$(DELAUNAY)". Use cgal or fade.)
# endif

# # Compiler settings
# CPP = g++ -std=c++17
# FLAGS = -O3 -march=native -lfreeglut -lGL -lGLU -lpthread $(DELAUNAY_FLAGS)

# # Specify file(s) to be compiled
# MAINFILE = example.cpp

# # Specify name of executable
# EXE = example

# .PHONY: all cls clean

# # Make arguments
# cls:
# 	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
# 	@clear
# 	@clear
# all:
# 	$(CPP) $(D_LIB) -o example $(ALL_LIB) $(MAINFILE) $(FLAGS)

# # Remove previously compiled executable
# clean:
# 	@rm -f $(EXE)

# Equivalent of [tasks.json](http://_vscodecontentref_/4) build task
# Usage:
#   mingw32-make
#   mingw32-make clean
#   mingw32-make SRC=your_file.cpp

CXX := C:/Program Files/CodeBlocks/MinGW/bin/g++.exe
WORKSPACE := C:/Users/s151044/CodeblocksProjects/BSO-toolbox-V2

# Match ${file} from tasks.json; overridable from command line
SRC ?= example.cpp

OUT := $(WORKSPACE)/main.exe

CXXFLAGS := -fdiagnostics-color=always -Wno-unknown-pragmas -g -std=c++17 -m64 -Og -O3 -DBSO_USE_CGAL

INCLUDES := \
 -I"$(WORKSPACE)" \
 -I"C:/Program Files/CodeBlocks/eigen-3.4.0" \
 -I"C:/Program Files/CodeBlocks/boost_1_81_0" \
 -I"$(WORKSPACE)/freeglut/include/GL" \
 -I"C:/Program Files/CodeBlocks/CGAL-6.1/include" \
 -I"C:/Program Files/CodeBlocks/CGAL-6.1/auxiliary/gmp/include"

LDFLAGS := \
 -L"C:/Program Files/CodeBlocks/boost_1_81_0/stage/lib" \
 -L"$(WORKSPACE)/freeglut/lib" \
 -L"C:/Program Files/CodeBlocks/CGAL-6.1/auxiliary/gmp/lib" \
 -lfreeglut -lopengl32 -lwinmm -lgdi32 -lgmp -lmpfr \
 -Wl,--subsystem,console

.PHONY: all clean

all:
	"$(CXX)" "$(SRC)" $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o "$(OUT)"

clean:
	del /Q "$(OUT)" 2>nul || exit 0