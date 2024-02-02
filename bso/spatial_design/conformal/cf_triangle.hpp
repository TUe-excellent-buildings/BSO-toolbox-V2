#ifndef CF_TRIANGLE_HPP
#define CF_TRIANGLE_HPP

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_triangle : public bso::utilities::geometry::triangle,
											 public cf_geometry_entity
	{
	private:
		
	public:
		cf_triangle(const utilities::geometry::triangle& rhs, cf_geometry_model* geomModel);
		~cf_triangle();
		
		void addLine					(cf_line* 			lPtr	) 	= delete;
		void addRectangle				(cf_rectangle* 		recPtr) 	= delete;
		void addTriangle				(cf_triangle* 		triPtr) 	= delete;
		void addCuboid					(cf_cuboid* 		tetPtr) 	= delete;
		void removeLine					(cf_line* 			lPtr	) 	= delete;
		void removeRectangle			(cf_rectangle* 		recPtr) 	= delete;
		void removeTriangle				(cf_triangle* 		triPtr) 	= delete;
		void removeCuboid				(cf_cuboid* 		tetPtr) 	= delete;
		void addPoint					(cf_point*			pPtr	) 	= delete;
		void addEdge					(cf_edge*			ePtr	) 	= delete;
		void addSpace					(cf_space*			spPtr	) 	= delete;
		
		const std::vector<cf_rectangle*		>& cfRectangles() 	= delete;
		const std::vector<cf_triangle*		>& cfTriangle()		= delete;
		const std::vector<cf_cuboid*		>& cfCuboids()		= delete;
		const std::vector<cf_point*			>& cfPoints() 		= delete;
		const std::vector<cf_edge*			>& cfEdges() 		= delete;
		const std::vector<cf_space*			>& cfSpaces() 		= delete;
		
		bool markInVisualization = false;
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_TRIANGLE_HPP