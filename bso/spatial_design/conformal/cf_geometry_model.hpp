#ifndef CF_GEOMETRY_MODEL_HPP
#define CF_GEOMETRY_MODEL_HPP

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_geometry_model
	{
	protected:
		double mTol;
		int mDec;
		
		std::vector<cf_vertex*> mCFVertices;
		std::vector<cf_line*> mCFLines;
		std::vector<cf_rectangle*> mCFRectangles;
		std::vector<cf_triangle*> mCFTriangles;
		std::vector<cf_cuboid*> mCFCuboids;
		std::vector<cf_triPrism*> mCFTriPrisms;
	public:
		cf_geometry_model(const double& tol = 1e-3);
		~cf_geometry_model();
		
		const double& tolerance() const {return mTol;}
		
		cf_vertex* addVertex(const bso::utilities::geometry::vertex& p);
		cf_line* addLine(const bso::utilities::geometry::line_segment& l);
		cf_rectangle* addRectangle(const bso::utilities::geometry::quadrilateral& quad);
		cf_triangle* addTriangle(const bso::utilities::geometry::triangle& tri);
		cf_cuboid* addCuboid(const bso::utilities::geometry::quad_hexahedron& qhex);
		cf_triPrism* addTriPrism(const bso::utilities::geometry::triangular_prism& triPtr);
		
		void removeLine(cf_line* lPtr);
		void removeRectangle(cf_rectangle* recPtr);
		void removeTriangle(cf_triangle* triPtr);
		void removeCuboid(cf_cuboid* cubPtr);
		void removeTriPrism(cf_triPrism* triPtr);
		
		const std::vector<cf_vertex*		>& cfVertices() 	const { return mCFVertices;}
		const std::vector<cf_line*			>& cfLines() 		const { return mCFLines;}
		const std::vector<cf_rectangle*		>& cfRectangles() 	const { return mCFRectangles;}
		const std::vector<cf_triangle*		>& cfTriangles() 	const { return mCFTriangles;}
		const std::vector<cf_cuboid*		>& cfCuboids() 		const { return mCFCuboids;}
		const std::vector<cf_triPrism*		>& cfTriPrism()	const { return mCFTriPrisms;}
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_GEOMETRY_MODEL_HPP