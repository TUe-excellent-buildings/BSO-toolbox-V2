#ifndef CF_CUBOID_HPP
#define CF_CUBOID_HPP

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_cuboid : public bso::utilities::geometry::quad_hexahedron,
										public cf_geometry_entity
	{
	private:
		
	public:
		cf_cuboid(const utilities::geometry::quad_hexahedron& rhs, cf_geometry_model* geomModel);
		~cf_cuboid();
		
		void split(cf_vertex* pPtr);
		void checkAssociated(cf_vertex* pPtr);
		
		void addLine				(cf_line* 			lPtr) 	= delete ;
		void addRectangle			(cf_rectangle* 		recPtr) = delete ;
		void addTriangle			(cf_triangle* 		triPtr) = delete ;
		void addCuboid				(cf_cuboid* 		cubPtr) = delete ;
		void addTriPrism			(cf_triPrism*		priPtr)	= delete ;
		void removeLine				(cf_line* 			lPtr) 	= delete ;
		void removeRectangle		(cf_rectangle* 		recPtr) = delete ;
		void removeTriangle			(cf_triangle* 		triPtr) = delete ;
		void removeCuboid			(cf_cuboid* 		cubPtr) = delete ;
		void removeTriPrism			(cf_triPrism*		priPtr)	= delete ;
		void addPoint				(cf_point*			pPtr) 	= delete ;
		void addEdge				(cf_edge*			ePtr) 	= delete ;
		void addSurface				(cf_surface*		srfPtr) = delete ;
		
		const std::vector<cf_triangle*		>& cfTriangles()	= delete;
		const std::vector<cf_cuboid*		>& cfCuboids() 		= delete;
		const std::vector<cf_triPrism*		>& cfTriPrism()		= delete;
		const std::vector<cf_point*			>& cfPoints() 		= delete;
		const std::vector<cf_edge*			>& cfEdges() 		= delete;
		const std::vector<cf_surface*		>& cfSurfaces() 	= delete;
		
		bool markInVisualization = false;
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_CUBOID_HPP