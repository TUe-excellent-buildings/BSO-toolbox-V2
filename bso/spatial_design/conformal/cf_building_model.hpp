#ifndef CF_BUILDING_MODEL_HPP
#define CF_BUILDING_MODEL_HPP

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <functional>

namespace bso { namespace spatial_design { namespace conformal {

	// Forward declaration of the default window geometry grammar tag, its specializations of
	// cf_building_model::windowGeometryGrammar()/windowGeometryGrammarND() are defined in a separate file,
	// see bso/spatial_design/conformal/window_geometry_grammars/default_window_geometry_grammar.cpp.
	// New window geometries can be added the same way: declare a new tag class, add a
	// specialization in a new file under window_geometry_grammars/, and #include it below calls definition.
	class DEFAULT_WINDOW_GEOMETRY_GRAMMAR;

	// Tag type used to select a window geometry grammar at construction time (constructors
	// cannot take explicit template arguments, so the grammar type is deduced from this tag).
	template <typename T>
	struct window_geometry_grammar_tag {};
	
	class cf_building_model : public cf_geometry_model
	{
	private:
		std::vector<cf_point*> mCFPoints;
		std::vector<cf_edge*> mCFEdges;
		std::vector<cf_surface*> mCFSurfaces;
		std::vector<cf_space*> mCFSpaces;

		// Iterative partition method functions for orthogonal cuboid BSDs
		void addSpace(const ms_space& msSpace); // defines an initial geometry conformal model to partition/ a finished building conformal model
		void addSpace(const ms_space& msSpace, std::vector<utilities::geometry::quad_hexahedron> quadHexa); // Add space with known conformal cuboids, no need for making it conformal!
		void makeConformal(); // orthogonal BSD partitioning method
		void makeConformal(std::vector<utilities::geometry::vertex> addConstrainVertexes); // orthogonal BSD partitioning method, considering additional constrains, for example for windows.
		
		// Triangulation partitioning method functions for non-orthogonal BSDs.
		void addSpaceD(const ms_space& msSpace, std::vector<utilities::geometry::triangular_prism> triPrismPtr); // Generates the final building- and geometry-conformal model after partitioning.
		void makeConformalND(const ms_N_building& msModel, const std::vector<utilities::geometry::vertex>& addConstrainVertexes = std::vector<utilities::geometry::vertex>()); // Initiates the triangulation partitioning method, optionally considering additional constrains (e.g., for windows).

		// Window grammars: read a window settings file, derive the window constraint vertices and
		// make the conformal model conformal to them. Only explicit specializations exist (defined
		// in files under window_grammars/), so selecting an unimplemented grammar fails to link.
		template <typename T>
		void windowGrammar(const std::string& fileNameWindowSettings); // for models based on ms_building (orthogonal BSDs)
		template <typename T>
		void windowGrammarND(const std::string& fileNameWindowSettings); // for models based on ms_N_building (non-orthogonal BSDs)

		// Safe it, in case copy constructor is called
		ms_building mMSModel; 
		ms_N_building mMSNModel; 
		std::vector<utilities::geometry::triangular_prism> mTriPrismInput;
		std::vector<utilities::geometry::quad_hexahedron> mQuadHexaInput;
		std::vector<bso::utilities::geometry::vertex> mConstrainVertexes;
		std::vector<bso::utilities::geometry::vertex> mAddDelaunayConstrains; // vertices which should be added as constrain points for the delaunay triangulation, for example to make sure that the partitioning is conformal to the window openings.
		std::string mfileNameDLSettings = ""; 	

		// Remembers which window grammar type was selected at construction, 
		// so the copy constructor can rebuild rhs with the same grammar.
		std::function<cf_building_model*(const cf_building_model&)> mWindowGrammarCloneFn = nullptr;

		// Stores the window constraint vertices for each space, either quadrilateral or triangular,
		// so the subsequent window grammar can identify which rectangles/triangles in the 
		// conformal model were originally defined for posibly being defined as a windows.
		std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> mWindowConstrainsPerSpaceQH;
		std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> mWindowConstrainsPerSpaceTP;

		double mTol;
		
		// safe it, in case of an iterm visualization the Triangulation partitioning method is required
		std::vector<utilities::geometry::quad_hexahedron> mSpacesAtGround; // spaces representation used to find intersection and constrain lines on the z=0 plane for 2D constrained delaunay aplication
		std::vector<utilities::geometry::line_segment> delauneyLines; //Lines where the constrained delaney triangulation should constrain to
		std::vector<utilities::geometry::triangle> delaunayTri; //resulting delaunay triangles at z=0 for triPrism definition in the geometry conformal model
		
		cf_building_model& operator = (cf_building_model& rhs) = default;
		friend class cf_geometry_entity;
	public:
		// Constructors for generating a conformal building model without any window geometry.
		cf_building_model(const cf_building_model& rhs); //copy constructor
		cf_building_model(const ms_building& msModel, const double& tol = 1e-3);
		cf_building_model(const ms_building& msModel, const std::vector<utilities::geometry::quad_hexahedron>& quadHexa, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in quad_hexehedrons
		cf_building_model(const ms_N_building& msNModel, const double& tol = 1e-3);
		cf_building_model(const ms_N_building& msNModel, const std::vector<utilities::geometry::triangular_prism>& triPrismPtr, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in triangular prims
		
		// Constructors for generating a conformal building model with windows geometry.
			// This is important as properties can only be assigned (performed in the grammar class) to the whole of a rectangle/triangle, 
			// and not to a part of it, so the window geometry needs to be reflected in the conformal building model.
			// Numerous grammars options can be developed for selecting the available window geometry to choose from.
			// To run either constructors with one of the available window geometry grammars, 
			// use the following syntax: cf_building_model(msModel, fileName, window_geometry_grammar_tag<my_window_grammar>());
			// for example, look to: /home/tessa/BSO-toolbox-V2/examples/NOBSD_basic/main.cpp.

		template <typename T>
		cf_building_model(const ms_building& msModel, const std::string& fileNameWindowSettings, window_geometry_grammar_tag<T>, const double& tol = 1e-3)
		: cf_geometry_model(tol), mMSModel(msModel), mTol(tol), mfileNameDLSettings(fileNameWindowSettings)
		{
			mWindowGrammarCloneFn = [](const cf_building_model& rhs) -> cf_building_model*
			{ return new cf_building_model(rhs.mMSModel, rhs.mfileNameDLSettings, window_geometry_grammar_tag<T>(), rhs.mTol); };
			this->windowGrammar<T>(fileNameWindowSettings);
		}
		template <typename T>
		cf_building_model(const ms_N_building& msNModel, const std::string& fileNameWindowSettings, window_geometry_grammar_tag<T>, const double& tol = 1e-3)
		: cf_geometry_model(tol), mMSNModel(msNModel), mTol(tol), mfileNameDLSettings(fileNameWindowSettings)
		{
			mWindowGrammarCloneFn = [](const cf_building_model& rhs) -> cf_building_model*
			{ return new cf_building_model(rhs.mMSNModel, rhs.mfileNameDLSettings, window_geometry_grammar_tag<T>(), rhs.mTol); };
			this->windowGrammarND<T>(fileNameWindowSettings);
		}
		
		~cf_building_model();		

		const std::vector<cf_point*			>& cfPoints() 		const { return mCFPoints;}
		const std::vector<cf_edge*			>& cfEdges() 			const { return mCFEdges;}
		const std::vector<cf_surface*		>& cfSurfaces() 	const { return mCFSurfaces;}
		const std::vector<cf_space*			>& cfSpaces() 		const { return mCFSpaces;}
		
		void writeToFile(std::string fileName, int writeOption = 1) const;

		// Functions used to allow for visualization of the interim stages of the Delaunay method
		const std::vector<utilities::geometry::quad_hexahedron> getSpacesAtGround() const {return mSpacesAtGround;} 
		const std::vector<utilities::geometry::line_segment> getDelauneyLines() const {return delauneyLines;} 
		const std::vector<utilities::geometry::triangle> getDelaunayTri() const {return delaunayTri;} 

		// Function to give access to window geometry contrain vertices for each space, 
		// either quadrilateral or triangular prism, so a window_grammar 
		// (in folder: /BSO-toolbox-V2/bso/grammar/window_grammars) 
		// can identify which rectangles/triangles geometry was originally generated to 
		// possibly become a windows or a wall.
		const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> getWindowConstrainsPerSpaceQH() {return mWindowConstrainsPerSpaceQH;}
		const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> getWindowConstrainsPerSpaceTP() {return mWindowConstrainsPerSpaceTP;}
		
		friend std::ostream& operator << (std::ostream& stream, const cf_building_model& building);
	};
	
} // conformal
} // spatial_design
} // bso

	#include <bso/spatial_design/conformal/window_geometry_grammars/default_window_geometry_grammar.cpp>

#endif // CF_BUILDING_MODEL_HPP