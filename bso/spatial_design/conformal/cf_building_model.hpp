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

		// Iterative partition method functions for orthogonal cuboid BSDs
		void addSpace(const ms_space& msSpace); // defines an initial geometry conformal model to partition and a finished building conformal model
		void addSpace(const ms_space& msSpace, std::vector<utilities::geometry::quad_hexahedron> quadHexa); // Add space with known conformal cuboids, no need for making it conformal!
		void makeConformal(); // orthogonal BSD partitioning method
		void makeConformal(std::vector<utilities::geometry::vertex> addConstrainVertexes); // ortogoal BSD paritioning method
		
		// Triangulation partitioning method functions for non-orthogonal BSDs (see public addSpaceD function for last function used for the method)
		void addSpaceD(const ms_space& msSpace, std::vector<utilities::geometry::triangular_prism> triPrismPtr); // Generates the final building- and geometry-conformal model after partitioning.
		void makeConformalND(const ms_N_building& msModel); // Initiates the triangulation partitioning method

		// safe it, in case copy constructor is called
		ms_building mMSModel; 
		ms_N_building mMSNModel; 
		std::vector<utilities::geometry::triangular_prism> triPrismInput;
		std::vector<utilities::geometry::quad_hexahedron> quadHexaInput;
		std::vector<bso::utilities::geometry::vertex> constrainVertexes;
		std::string mfileNameDLSettings = ""; 
		std::vector<std::tuple<unsigned int, std::vector<bso::utilities::geometry::vertex>>> ConstrainsPerSpaceQH;

		double mTol;
		
		// safe it, in case of an iterm visualization the Triangulation partitioning method is required
		std::vector<utilities::geometry::quad_hexahedron> spacesAtGround; // spaces representation used to find intersection and constrain lines on the z=0 plane for 2D constrained delaunay aplication
		std::vector<utilities::geometry::line_segment> delauneyLines; //Lines where the constrained delaney triangulation should constrain to
		std::vector<utilities::geometry::triangle> delaunayTri; //resulting delaunay triangles at z=0 for triPrism definition in the geometry conformal model
		
		cf_building_model& operator = (cf_building_model& rhs) = default;
		friend class cf_geometry_entity;
	public:
		cf_building_model(const cf_building_model& rhs); //copy constructor
		cf_building_model(const ms_building& msModel, const double& tol = 1e-3);
		cf_building_model(const ms_building& msModel, const std::string& fileNameDLSettings, const double& tol = 1e-3);
		cf_building_model(const ms_building& msModel, const std::vector<utilities::geometry::quad_hexahedron>& quadHexa, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in quad_hexehedrons
		cf_building_model(const ms_N_building& msModel, const double& tol = 1e-3);
		cf_building_model(const ms_N_building& msModel, const std::vector<utilities::geometry::triangular_prism>& triPrismPtr, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in triangular prims
		~cf_building_model();		

		const std::vector<cf_point*			>& cfPoints() 		const { return mCFPoints;}
		const std::vector<cf_edge*			>& cfEdges() 			const { return mCFEdges;}
		const std::vector<cf_surface*		>& cfSurfaces() 	const { return mCFSurfaces;}
		const std::vector<cf_space*			>& cfSpaces() 		const { return mCFSpaces;}
		
		void writeToFile(std::string fileName, int writeOption = 1) const;

		// Functions used to allow for visualization of the interim stages of the Delaunay method
		const std::vector<utilities::geometry::quad_hexahedron> getSpacesAtGround() const {return spacesAtGround;} 
		const std::vector<utilities::geometry::line_segment> getDelauneyLines() const {return delauneyLines;} 
		const std::vector<utilities::geometry::triangle> getDelaunayTri() const {return delaunayTri;} 

		//function to give access to 
		const std::vector<std::tuple<unsigned int, std::vector<bso::utilities::geometry::vertex>>> getConstrainsPerSpaceQH() {return ConstrainsPerSpaceQH;}
		
		friend std::ostream& operator << (std::ostream& stream, const cf_building_model& building);
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_BUILDING_MODEL_HPP