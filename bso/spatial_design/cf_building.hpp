#ifndef CF_BUILDING_HPP
#define CF_BUILDING_HPP

#include <bso/utilities/geometry.hpp>

// declarations
namespace bso { namespace spatial_design { namespace conformal {
	class cf_entitiy;
	class cf_geometry_model;
	class cf_building_model;
	class cf_geometry_entity;
	class cf_vertex;
	class cf_line;
	class cf_rectangle;
	class cf_triangle;
	class cf_cuboid;
	class cf_triPrism;
	class cf_building_entity;
	class cf_point;
	class cf_edge;
	class cf_surface;
	class cf_space;
} // namespace conformal
} // namespace spatial_design
} // namespace bso

// class definitions
#include <bso/spatial_design/conformal/cf_geometry_model.hpp>
#include <bso/spatial_design/conformal/cf_building_model.hpp>
#include <bso/spatial_design/conformal/cf_entity.hpp>
#include <bso/spatial_design/conformal/cf_geometry_entity.hpp>
#include <bso/spatial_design/conformal/cf_building_entity.hpp>
#include <bso/spatial_design/conformal/cf_vertex.hpp>
#include <bso/spatial_design/conformal/cf_line.hpp>
#include <bso/spatial_design/conformal/cf_rectangle.hpp>
#include <bso/spatial_design/conformal/cf_triangle.hpp>
#include <bso/spatial_design/conformal/cf_cuboid.hpp>
#include <bso/spatial_design/conformal/cf_triprism.hpp>
#include <bso/spatial_design/conformal/cf_building_entity.hpp>
#include <bso/spatial_design/conformal/cf_point.hpp>
#include <bso/spatial_design/conformal/cf_edge.hpp>
#include <bso/spatial_design/conformal/cf_surface.hpp>
#include <bso/spatial_design/conformal/cf_space.hpp>

// class implementations
#include <bso/spatial_design/conformal/cf_geometry_model.cpp>
#include <bso/spatial_design/conformal/cf_building_model.cpp>
#include <bso/spatial_design/conformal/cf_entity.cpp>
#include <bso/spatial_design/conformal/cf_geometry_entity.cpp>
#include <bso/spatial_design/conformal/cf_building_entity.cpp>
#include <bso/spatial_design/conformal/cf_vertex.cpp>
#include <bso/spatial_design/conformal/cf_line.cpp>
#include <bso/spatial_design/conformal/cf_rectangle.cpp>
#include <bso/spatial_design/conformal/cf_triangle.cpp>
#include <bso/spatial_design/conformal/cf_cuboid.cpp>
#include <bso/spatial_design/conformal/cf_triprism.cpp>
#include <bso/spatial_design/conformal/cf_building_entity.cpp>
#include <bso/spatial_design/conformal/cf_point.cpp>
#include <bso/spatial_design/conformal/cf_edge.cpp>
#include <bso/spatial_design/conformal/cf_surface.cpp>
#include <bso/spatial_design/conformal/cf_space.cpp>

namespace bso { namespace spatial_design {
	typedef conformal::cf_building_model cf_building;
// cf_building now refers to the namespaces conformal::cf_building_model, 
// so look in the conformal directory to the file "cf_building_model" for the function used.
} // namespace spatial_design
} // namespace bso
	
#endif // CF_BUILDING_HPP