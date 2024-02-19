#ifndef CF_TRIPRISM_HPP
#define CF_TRIPRISM_HPP

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_triPrism : public bso::utilities::geometry::triangular_prism,
										public cf_geometry_entity
	{
	private:
		double areaSide1;
		double areaSide2;
		double areaSide3;
		double areaBase1;
		double areaBase2;
		double totalArea;

		double typeSide1 = 0;
		double typeSide2 = 0;
		double typeSide3 = 0;
		double typeBase1 = 0;
		double typeBase2 = 0;
		
	public:
		cf_triPrism(const utilities::geometry::triangular_prism& rhs, cf_geometry_model* geometryModel);
		~cf_triPrism();
		
		void addLine					(cf_line* 			lPtr) 		= delete ;
		void addRectangle				(cf_rectangle* 		recPtr) 	= delete ;
		void addTriangle				(cf_triangle* 		triPtr) 	= delete;
		void addCuboid					(cf_cuboid* 		cubPtr) 	= delete ;
		void addTriPrism				(cf_triPrism* 		priPtr)		= delete ;
		void removeLine					(cf_line* 			lPtr) 		= delete ;
		void removeRectangle			(cf_rectangle* 		recPtr) 	= delete ;
		void removeTriangle				(cf_triangle* 		triPtr) 	= delete;
		void removeCuboid				(cf_cuboid* 		cubPtr) 	= delete ;
		void addPoint					(cf_point*			pPtr) 		= delete ;
		void addEdge					(cf_edge*			ePtr) 		= delete ;
		void addSurface					(cf_surface*		srfPtr) 	= delete ;
		
		const std::vector<cf_rectangle*		>& cfRectangles() 	= delete;
		const std::vector<cf_triPrism*		>& cfTriPrism() 	= delete;
		const std::vector<cf_cuboid*		>& cfCuboids() 		= delete;
		const std::vector<cf_point*			>& cfPoints() 		= delete;
		const std::vector<cf_edge*			>& cfEdges() 		= delete;
		const std::vector<cf_surface*		>& cfSurfaces() 	= delete;

		bool operator == (const cf_triPrism& rhs) const;
		bool operator != (const cf_triPrism& rhs) const;

		void setAreas(double areaSide1, double areaSide2, double areaSide3);
		std::vector<double> getAreas() { return {areaSide1, areaSide2, areaSide3}; } 
		std::vector<double> getTypes() { return {typeSide1, typeSide2, typeSide3}; }
		double getTotalArea() { return totalArea; }
	};
	

} // conformal
} // spatial_design
} // bso

#endif // CF_TRIPRISM_HPP