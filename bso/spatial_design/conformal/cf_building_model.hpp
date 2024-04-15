#ifndef CF_BUILDING_MODEL_HPP
#define CF_BUILDING_MODEL_HPP

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_building_model : public cf_geometry_model
	{
	private:
		std::vector<cf_point*> mCFPoints;
		std::vector<cf_edge*> mCFEdges;
		std::vector<cf_surface*> mCFSurfaces;
		std::vector<cf_space*> mCFSpaces;
		ms_building mMSModel; // safe it, in case copy consttructor is called
		ms_N_building mMSNModel; // safe it, in case copy consttructor is called
		std::vector<bso::utilities::geometry::vertex> constrainVertexes; // safe it, in case copy constructor is called
		double mTol;
		
		// Iterative partition method functions for orthogonal cuboid BSDs
		void addSpace(const ms_space& msSpace); // defines an initial geometry conformal model to partition and a finished building conformal model
		void makeConformal(); // orthogonal BSD partitioning method
		void makeConformal(std::vector<utilities::geometry::vertex> addConstrainVertexes); // orthogonal BSD partitioning method
		
		// Triangulation partitioning method functions for non-orthogonal BSDs
		void makeConformalND(const ms_N_building& msModel); // Initiates the triangulation partitioning method
		std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delauneyPoints, const std::vector <utilities::geometry::line_segment> delauneyLines); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
		void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines
		void addSpaceD(const ms_space& msSpace, std::vector<utilities::geometry::triangular_prism>& triPrismPtr); // Generates the final buiding- and geometry-conformal model after partitioning.
		
		// safe it, in case of an intrem visualisation the Triangulation partitioning method is required
		std::vector<utilities::geometry::quad_hexahedron> spacesAtGround; // spaces representation used to find intersection and constrain lines on the z=0 plane for 2D constrained delaunay aplication
		std::vector<utilities::geometry::line_segment> delauneyLines; //Lines where the constrained delauney triangulation should constrain to
		std::vector<utilities::geometry::triangle> delaunayTri; //resulting delaunay triangles at z=0 for triPrism definition in the geometry conformal model
		
		cf_building_model& operator = (cf_building_model& rhs) = default;
		friend class cf_geometry_entity;
	public:
		cf_building_model(const cf_building_model& rhs);
		cf_building_model(const ms_building& msModel, const double& tol = 1e-3);
		cf_building_model(const ms_building& msModel, std::vector<bso::utilities::geometry::vertex> addConstrainVertexes, const double& tol = 1e-3);
		cf_building_model(const ms_N_building& msModel, const double& tol = 1e-3);
		~cf_building_model();

		const std::vector<cf_point*			>& cfPoints() 		const { return mCFPoints;}
		const std::vector<cf_edge*			>& cfEdges() 			const { return mCFEdges;}
		const std::vector<cf_surface*		>& cfSurfaces() 	const { return mCFSurfaces;}
		const std::vector<cf_space*			>& cfSpaces() 		const { return mCFSpaces;}
		
		void writeToFile(std::string fileName) const;

		//Functions used to allow for visualization of the interim stages of the Delaunay method
		const std::vector<utilities::geometry::quad_hexahedron> getSpacesAtGround() const {return spacesAtGround;} 
		const std::vector<utilities::geometry::line_segment> getDelauneyLines() const {return delauneyLines;} 
		const std::vector<utilities::geometry::triangle> getDelaunayTri() const {return delaunayTri;} 

		friend std::ostream& operator<< (std::ostream& stream, const cf_building_model& building);
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_BUILDING_MODEL_HPP