#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
BSO     = ..
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# set DELAUNAY=cgal (default) or DELAUNAY=fade when invoking make to choose the Delaunay triangulation backend
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
CPP   = g++ -std=c++17
FLAGS  = -O -march=native -lboost_unit_test_framework -lglut -lGL -lGLU -lpthread $(DELAUNAY_FLAGS)

#test suites
MS_SPACE    = $(BSO)/unit_tests/spatial_design/ms_space_test.cpp
MS_BUILDING = $(BSO)/unit_tests/spatial_design/ms_building_test.cpp
MS_N_BUILDING = $(BSO)/unit_tests/spatial_design/ms_N_building_test.cpp
SC_BUILDING = $(BSO)/unit_tests/spatial_design/sc_building_test.cpp
CONFORMAL 	= $(BSO)/unit_tests/spatial_design/conformal_test.cpp
TRIM_CAST   = $(BSO)/unit_tests/utilities/trim_and_cast_test.cpp
GEOMETRY	= $(BSO)/unit_tests/utilities/geometry_test.cpp
DELAUNAY	= $(BSO)/unit_tests/utilities/delaunay_test.cpp
EXPORT		= $(BSO)/unit_tests/utilities/data_export_test.cpp
STRUCT_DES	= $(BSO)/unit_tests/structural_design/structural_design_test.cpp
BUILD_PHYS  = $(BSO)/unit_tests/building_physics/building_physics_test.cpp
VISUALIZE   = $(BSO)/unit_tests/visualization/visualization_test.cpp
BSO_ALL		= $(BSO)/unit_tests/all_test.cpp
XML			= $(BSO)/unit_tests/spatial_design/xml/xml_test.cpp
DATA		= $(BSO)/unit_tests/utilities/data_handling_test.cpp
DATA_ANALYSIS = $(BSO)/unit_tests/utilities/data_analysis_test.cpp
GRAMMAR		= $(BSO)/unit_tests/grammar/grammar_test.cpp

.PHONY: all ms_space ms_building sc_building conformal trim_cast geometry delaunay plot building_physics structural_design clean visualization xml data grammar

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) $(D_LIB) -o all_test $(ALL_LIB) $(BSO_ALL) $(FLAGS)
ms_space:
	$(CPP) $(D_LIB) -o ms_space_test $(ALL_LIB) $(MS_SPACE) $(FLAGS)
ms_building:
	$(CPP) $(D_LIB) -o ms_building_test $(ALL_LIB) $(MS_BUILDING) $(FLAGS)
ms_N_building:
	$(CPP) $(D_LIB) -o ms_N_building_test $(ALL_LIB) $(MS_N_BUILDING) $(FLAGS)
sc_building:
	$(CPP) $(D_LIB) -o sc_building_test $(ALL_LIB) $(SC_BUILDING) $(FLAGS)
conformal:
	$(CPP) $(D_LIB) -o conformal_test $(ALL_LIB) $(CONFORMAL) $(FLAGS)
trim_cast:
	$(CPP) $(D_LIB) -o trim_cast_test $(ALL_LIB) $(TRIM_CAST) $(FLAGS)
geometry:
	$(CPP) $(D_LIB) -o geometry_test $(ALL_LIB) $(GEOMETRY) $(FLAGS)
delaunay:
	$(CPP) $(D_LIB) -o delaunay_test $(ALL_LIB) $(DELAUNAY) $(FLAGS)
plot:
	$(CPP) $(D_LIB) -o data_plot_test $(ALL_LIB) $(EXPORT) $(FLAGS)
structural_design:
	$(CPP) $(D_LIB) -o sd_test $(ALL_LIB) $(STRUCT_DES) $(FLAGS)
building_physics:
	$(CPP) $(D_LIB) -o bp_test $(ALL_LIB) $(BUILD_PHYS) $(FLAGS)
visualization:
	$(CPP) $(D_LIB) -o vis_test $(ALL_LIB) $(VISUALIZE) $(FLAGS)
xml:
	$(CPP) $(D_LIB) -o xml_test $(ALL_LIB) $(XML) $(FLAGS)
data:
	$(CPP) $(D_LIB) -o data_test $(ALL_LIB) $(DATA) $(FLAGS)
data_analysis:
	$(CPP) $(D_LIB) -o data_analysis_test $(ALL_LIB) $(DATA_ANALYSIS) $(FLAGS)
grammar:
	$(CPP) $(D_LIB) -o grammar_test $(ALL_LIB) $(GRAMMAR) $(FLAGS)
clean:
	@rm -f ms_space_test
	@rm -f ms_building_test
	@rm -f ms_N_building_test
	@rm -f sc_building_test
	@rm -f conformal_test
	@rm -f trim_cast_test
	@rm -f geometry_test
	@rm -f delaunay_test
	@rm -f data_plot_test
	@rm -f sd_test
	@rm -f bp_test
	@rm -f all_test
	@rm -f vis_test
	@rm -f xml_test
	@rm -f data_test
	@rm -f data_analysis_test
	@rm -f grammar_test