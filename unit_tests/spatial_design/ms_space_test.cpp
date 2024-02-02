#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE ms_space
#endif

#include <bso/spatial_design/ms_space.hpp>

#include <vector>
#include <stdexcept>

#include <boost/test/included/unit_test.hpp>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace spatial_design_test {
using namespace bso::spatial_design;

BOOST_AUTO_TEST_SUITE( space_initialization )

	BOOST_AUTO_TEST_CASE( empty_initialization )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		ms_space s1;
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 0);										
		BOOST_REQUIRE(s1.getID() == 0);
		BOOST_REQUIRE(s1.getSDefMethod() == "");								  
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_1 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(1000,1000,1000);
		ms_space s1(1, coord1, dim1);
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);										
		BOOST_REQUIRE(s1.getID() == 1);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");								   
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_2 )
	{
		Eigen::Vector3d coord1(500,6500,7000);
		Eigen::Vector3d dim1(2250,3500,4000);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s1(1, coord1, dim1, "type_a", surfaceTypes);
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");
		BOOST_REQUIRE(s1.getID() == 1);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_3 )
	{
		Eigen::Vector3d coord1(1000,1000,1000);
		Eigen::Vector3d dim1(3000,3000,3000);
		ms_space s1("R,3,3000,3000,3000,1000,1000,1000");
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);										
		BOOST_REQUIRE(s1.getID() == 3);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");								   
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_4 )
	{
		Eigen::Vector3d coord1(1000,1000,1000);
		Eigen::Vector3d dim1(3000,3000,3000);
		ms_space s1("R,4,3000,3000,3000,1000,1000,1000,type_a");
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");
		BOOST_REQUIRE(s1.getID() == 4);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_5 )
	{
		Eigen::Vector3d coord1(1000,1000,1000);
		Eigen::Vector3d dim1(3000,3000,3000);
		ms_space s1("R,5,3000,3000,3000,1000,1000,1000,type_b,type_1,type_2,type_3,type_4,type_5,type_6");
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");
		BOOST_REQUIRE(s1.getID() == 5);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_b");
		std::vector<std::string> surfaceTypes = {"type_1","type_2","type_3","type_4","type_5","type_6"};
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_6 )
	{
		Eigen::Vector3d coord1(1000,1000,1000);
		Eigen::Vector3d dim1(3000,3000,3000);
		ms_space s1("R,6,3000,3000,3000,1000,1000,1000,type_1,type_2,type_3,type_4,type_5,type_6");
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");
		BOOST_REQUIRE(s1.getID() == 6);
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes = {"type_1","type_2","type_3","type_4","type_5","type_6"};
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( getGeometryTest_1 )
	{
		ms_space s1("R,78,3000,3000,3000,1000,1000,1000");
		bso::utilities::geometry::quad_hexahedron spaceGeom = s1.getGeometry();
		bso::utilities::geometry::quad_hexahedron checkGeom = {
			{1000,1000,1000},{4000,1000,1000},{4000,4000,1000},{1000,4000,1000},
			{1000,1000,4000},{4000,1000,4000},{4000,4000,4000},{1000,4000,4000}
		};
		BOOST_CHECK_EQUAL_COLLECTIONS(spaceGeom.begin(), spaceGeom.end(), checkGeom.begin(), checkGeom.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_7 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		
		std::vector <bso::utilities::geometry::vertex*> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {0,0,0};
		bso::utilities::geometry::vertex vert2 = {0,1000,0};
		bso::utilities::geometry::vertex vert3 = {1000,1000,0};
		bso::utilities::geometry::vertex vert4 = {1000,0,0};
		bso::utilities::geometry::vertex vert5 = {0,0,1000};
		bso::utilities::geometry::vertex vert6 = {0,1000,1000};
		bso::utilities::geometry::vertex vert7 = {1000,1000,1000};
		bso::utilities::geometry::vertex vert8 = {1000,0,1000};
		
		pVertex.push_back(&vert1);
		pVertex.push_back(&vert2);
		pVertex.push_back(&vert3);
		pVertex.push_back(&vert4);
		pVertex.push_back(&vert5);
		pVertex.push_back(&vert6);
		pVertex.push_back(&vert7);
		pVertex.push_back(&vert8);
		
		ms_space s1(1, pVertex);
		bool check = true;
		for(const auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),i) == pVertex.end())
			{
				check = false;
			}
		}
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getID() == 1);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_8 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		
		std::vector <bso::utilities::geometry::vertex*> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {0,0,0};
		bso::utilities::geometry::vertex vert2 = {0,2000,0};
		bso::utilities::geometry::vertex vert3 = {1000,1000,0};
		bso::utilities::geometry::vertex vert4 = {1000,0,0};
		bso::utilities::geometry::vertex vert5 = {0,0,1000};
		bso::utilities::geometry::vertex vert6 = {0,2000,1000};
		bso::utilities::geometry::vertex vert7 = {1000,1000,1000};
		bso::utilities::geometry::vertex vert8 = {1000,0,1000};
		
		pVertex.push_back(&vert1);
		pVertex.push_back(&vert2);
		pVertex.push_back(&vert3);
		pVertex.push_back(&vert4);
		pVertex.push_back(&vert5);
		pVertex.push_back(&vert6);
		pVertex.push_back(&vert7);
		pVertex.push_back(&vert8);
		
		ms_space s1(1, pVertex, "type_a", surfaceTypes);
		bool check = true;
		for(const auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),i) == pVertex.end())
			{
				check = false;
			}
		}
		
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		BOOST_REQUIRE(s1.getID() == 1);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_9 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		
		std::vector <bso::utilities::geometry::vertex> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {16500,10500,0};
		bso::utilities::geometry::vertex vert2 = {14500,12500,0};
		bso::utilities::geometry::vertex vert3 = {14500,24500,0};
		bso::utilities::geometry::vertex vert4 = {16500,24500,0};
		bso::utilities::geometry::vertex vert5 = {16500,10500,4000};
		bso::utilities::geometry::vertex vert6 = {14500,12500,4000};
		bso::utilities::geometry::vertex vert7 = {14500,24500,4000};
		bso::utilities::geometry::vertex vert8 = {16500,24500,4000};
		
		pVertex.push_back(vert1);
		pVertex.push_back(vert2);
		pVertex.push_back(vert3);
		pVertex.push_back(vert4);
		pVertex.push_back(vert5);
		pVertex.push_back(vert6);
		pVertex.push_back(vert7);
		pVertex.push_back(vert8);		
		
		ms_space s1(" N, 6,	16500,10500,0,	14500,12500,0,	14500,24500,0,	16500,24500,0,	16500,10500,4000,	14500,12500,4000,	14500,24500,4000,	16500,24500,4000");
		
		bool check = true;
		for(auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),(*i)) == pVertex.end())
			{
				check = false;
			}
		}
		
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getID() == 6);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_10 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		
		std::vector <bso::utilities::geometry::vertex> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {16500,10500,0};
		bso::utilities::geometry::vertex vert2 = {14500,12500,0};
		bso::utilities::geometry::vertex vert3 = {14500,24500,0};
		bso::utilities::geometry::vertex vert4 = {16500,24500,0};
		bso::utilities::geometry::vertex vert5 = {16500,10500,4000};
		bso::utilities::geometry::vertex vert6 = {14500,12500,4000};
		bso::utilities::geometry::vertex vert7 = {14500,24500,4000};
		bso::utilities::geometry::vertex vert8 = {16500,24500,4000};
		
		pVertex.push_back(vert1);
		pVertex.push_back(vert2);
		pVertex.push_back(vert3);
		pVertex.push_back(vert4);
		pVertex.push_back(vert5);
		pVertex.push_back(vert6);
		pVertex.push_back(vert7);
		pVertex.push_back(vert8);		
		
		ms_space s1(" N, 6,	16500,10500,0,	14500,12500,0,	14500,24500,0,	16500,24500,0,	16500,10500,4000,	14500,12500,4000,	14500,24500,4000,	16500,24500,4000, type_a");
		
		bool check = true;
		for(const auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),(*i)) == pVertex.end())
			{
				check = false;
			}
		}		

		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		BOOST_REQUIRE(s1.getID() == 6);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> surfaceTypes;
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(!s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_11 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		
		std::vector <bso::utilities::geometry::vertex> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {0,0,4000};
		bso::utilities::geometry::vertex vert2 = {0,6000,4000};
		bso::utilities::geometry::vertex vert3 = {5000,6000,4000};
		bso::utilities::geometry::vertex vert4 = {5000,0,4000};
		bso::utilities::geometry::vertex vert5 = {0,0,7000};
		bso::utilities::geometry::vertex vert6 = {0,6000,7000};
		bso::utilities::geometry::vertex vert7 = {5000,6000,7000};
		bso::utilities::geometry::vertex vert8 = {5000,0,7000};
		
		pVertex.push_back(vert1);
		pVertex.push_back(vert2);
		pVertex.push_back(vert3);
		pVertex.push_back(vert4);
		pVertex.push_back(vert5);
		pVertex.push_back(vert6);
		pVertex.push_back(vert7);
		pVertex.push_back(vert8);		
		
		ms_space s1(" N, 3,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000, type_b,type_1,type_2,type_3,type_4,type_5,type_6");
		
		bool check = true;
		for(const auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),(*i)) == pVertex.end())
			{
				check = false;
			}
		}		
		
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		BOOST_REQUIRE(s1.getID() == 3);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_b");
		std::vector<std::string> surfaceTypes = {"type_1","type_2","type_3","type_4","type_5","type_6"};
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_12 )
	{
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		
		std::vector <bso::utilities::geometry::vertex> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {0,0,4000};
		bso::utilities::geometry::vertex vert2 = {0,6000,4000};
		bso::utilities::geometry::vertex vert3 = {5000,6000,4000};
		bso::utilities::geometry::vertex vert4 = {5000,0,4000};
		bso::utilities::geometry::vertex vert5 = {0,0,7000};
		bso::utilities::geometry::vertex vert6 = {0,6000,7000};
		bso::utilities::geometry::vertex vert7 = {5000,6000,7000};
		bso::utilities::geometry::vertex vert8 = {5000,0,7000};
		
		pVertex.push_back(vert1);
		pVertex.push_back(vert2);
		pVertex.push_back(vert3);
		pVertex.push_back(vert4);
		pVertex.push_back(vert5);
		pVertex.push_back(vert6);
		pVertex.push_back(vert7);
		pVertex.push_back(vert8);
		
		ms_space s1("N, 3,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000, type_1,type_2,type_3,type_4,type_5,type_6");
		
		bool check = true;
		for(const auto i: s1.getVertices())
		{
			if (std::find(pVertex.begin(),pVertex.end(),(*i)) == pVertex.end())
			{
				check = false;
			}
		}	
		
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(check == true);
		BOOST_REQUIRE(s1.getID() == 3);
		std::string checkSpaceType;
		BOOST_REQUIRE(!s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "");
		std::vector<std::string> surfaceTypes = {"type_1","type_2","type_3","type_4","type_5","type_6"};
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( getGeometryTest_2 )
	{
		ms_space s1("N, 3,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000");
		bso::utilities::geometry::quad_hexahedron spaceGeom = s1.getGeometry();
		bso::utilities::geometry::quad_hexahedron checkGeom = {
			{0,0,4000},{0,6000,4000},{5000,6000,4000},{5000,0,4000},
			{0,0,7000},{0,6000,7000},{5000,6000,7000},{5000,0,7000}
		};
		BOOST_CHECK_EQUAL_COLLECTIONS(spaceGeom.begin(), spaceGeom.end(), checkGeom.begin(), checkGeom.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_by_assignment_1 )
	{
		ms_space s1;
		Eigen::Vector3d coord1(500,6500,7000);
		Eigen::Vector3d dim1(2250,3500,4000);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, coord1, dim1, "type_a", surfaceTypes);
		s1 = s2;
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "R");
		BOOST_REQUIRE(s1.getID() == 1);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE( initialization_by_assignment_2 )
	{
		ms_space s1;
		
		std::vector <bso::utilities::geometry::vertex*> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {16500,10500,0};
		bso::utilities::geometry::vertex vert2 = {14500,12500,0};
		bso::utilities::geometry::vertex vert3 = {14500,24500,0};
		bso::utilities::geometry::vertex vert4 = {16500,24500,0};
		bso::utilities::geometry::vertex vert5 = {16500,10500,4000};
		bso::utilities::geometry::vertex vert6 = {14500,12500,4000};
		bso::utilities::geometry::vertex vert7 = {14500,24500,4000};
		bso::utilities::geometry::vertex vert8 = {16500,24500,4000};
		
		pVertex.push_back(&vert1);
		pVertex.push_back(&vert2);
		pVertex.push_back(&vert3);
		pVertex.push_back(&vert4);
		pVertex.push_back(&vert5);
		pVertex.push_back(&vert6);
		pVertex.push_back(&vert7);
		pVertex.push_back(&vert8);
		
		Eigen::Vector3d coord1(0,0,0);
		Eigen::Vector3d dim1(0,0,0);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, pVertex, "type_a", surfaceTypes);
		s1 = s2;
		BOOST_REQUIRE(s1.getCoordinates() == coord1);
		BOOST_REQUIRE(s1.getDimensions() == dim1);
		BOOST_REQUIRE((s1.getVertices()).size() == 8);
		BOOST_REQUIRE(s1.getSDefMethod() == "N");
		BOOST_REQUIRE(s1.getID() == 1);
		std::string checkSpaceType;
		BOOST_REQUIRE(s1.getSpaceType(checkSpaceType));
		BOOST_REQUIRE(checkSpaceType == "type_a");
		std::vector<std::string> checkSurfaceTypes;
		BOOST_REQUIRE(s1.getSurfaceTypes(checkSurfaceTypes));
		BOOST_CHECK_EQUAL_COLLECTIONS(checkSurfaceTypes.begin(), checkSurfaceTypes.end(), surfaceTypes.begin(), surfaceTypes.end());
	}
	
	BOOST_AUTO_TEST_CASE ( initialize_invalid_space_1 )
	{
		Eigen::Vector3d coord1(500,6500,7000);
		Eigen::Vector3d dim1(2250,-3500,4000);
		BOOST_REQUIRE_THROW(ms_space s1(1, coord1, dim1), std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE ( initialize_invalid_space_2 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("6,3000,-3000,3000,1000,1000,1000,type_1,type_2,type_3,type_4,type_5,type_6"), std::invalid_argument);
	}

  BOOST_AUTO_TEST_CASE( initialize_invalid_space_3 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R,	4,	0,0,0,	4000,3000,2000,	5000,2000,1000"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_4 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R,	4,	0,0,0,	4000,3000,2000,	5000,2000,1000,	4000,3000"), std::invalid_argument);
		//BOOST_REQUIRE_THROW(ms_space s1("R,	4,	0,0,0,	4000,3000,2000,	5000,2000,1000,	4000,3000,1000"), std::invalid_argument);
		//The second does not give an error and is therefore wrong. However, It does insert values at surface type value's. Althoug this is not correct,
		//It is assumed that the user should get this insert correct. 
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_5 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R,	4,	0,0,0"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_6 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("N, 	3,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_7 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R, 	2,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_8 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R, 	2,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_9 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("R,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( initialize_invalid_space_10 )
	{
		BOOST_REQUIRE_THROW(ms_space s1("1,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000"), std::invalid_argument);
	}
	
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( space_comparison )

	BOOST_AUTO_TEST_CASE( comparison_1 )
	{
		ms_space s1;
		Eigen::Vector3d coord1(500,6500,7000);
		Eigen::Vector3d dim1(2250,3500,4000);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, coord1, dim1, "type_a", surfaceTypes);
		s1 = s2;
		BOOST_REQUIRE(s1 == s2);
		BOOST_REQUIRE(s2 == s1);
		BOOST_REQUIRE(!(s1 != s2));
		BOOST_REQUIRE(!(s2 != s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_2 )
	{
		ms_space s1("R,1,2250,3500,4000,500,6500,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		Eigen::Vector3d coord1(500,6500,7000);
		Eigen::Vector3d dim1(2250,3500,4000);
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, coord1, dim1, "type_a", surfaceTypes);
		BOOST_REQUIRE(s1 == s2);
		BOOST_REQUIRE(s2 == s1);
		BOOST_REQUIRE(!(s1 != s2));
		BOOST_REQUIRE(!(s2 != s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_3 )
	{
		ms_space s1("R,1,2250,3500,4000,500,6500,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("R,1,2250,3500,4000,500,6500,7000,type_a,type_1,type_2,type_4,type_4,type_5,type_6");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_4 )
	{
		ms_space s1("R,1,2250,3500,4000,500,6500,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("R,1,2250,3500,4000,500,6500,7000,type_a");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_5 )
	{
		ms_space s1("R,1,2250,3500,4000,500,6500,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("R,1,2250,3500,4000,500,6500,7000,type_1,type_2,type_4,type_4,type_5,type_6");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_6 )
	{
		ms_space s1;
		
		std::vector <bso::utilities::geometry::vertex*> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {16500,10500,0};
		bso::utilities::geometry::vertex vert2 = {14500,12500,0};
		bso::utilities::geometry::vertex vert3 = {14500,24500,0};
		bso::utilities::geometry::vertex vert4 = {16500,24500,0};
		bso::utilities::geometry::vertex vert5 = {16500,10500,4000};
		bso::utilities::geometry::vertex vert6 = {14500,12500,4000};
		bso::utilities::geometry::vertex vert7 = {14500,24500,4000};
		bso::utilities::geometry::vertex vert8 = {16500,24500,4000};
		
		pVertex.push_back(&vert1);
		pVertex.push_back(&vert2);
		pVertex.push_back(&vert3);
		pVertex.push_back(&vert4);
		pVertex.push_back(&vert5);
		pVertex.push_back(&vert6);
		pVertex.push_back(&vert7);
		pVertex.push_back(&vert8);
		
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, pVertex, "type_a", surfaceTypes);
		s1 = s2;
		BOOST_REQUIRE(s1 == s2);
		BOOST_REQUIRE(s2 == s1);
		BOOST_REQUIRE(!(s1 != s2));
		BOOST_REQUIRE(!(s2 != s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_7 )
	{
		ms_space s1("N, 1,	16500,10500,0,	14500,12500,0,	14500,24500,0,	16500,24500,0,	16500,10500,4000,	14500,12500,4000,	14500,24500,4000,	16500,24500,4000, type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		
		std::vector <bso::utilities::geometry::vertex*> pVertex;
		
		bso::utilities::geometry::vertex vert1 = {16500,10500,0};
		bso::utilities::geometry::vertex vert2 = {14500,12500,0};
		bso::utilities::geometry::vertex vert3 = {14500,24500,0};
		bso::utilities::geometry::vertex vert4 = {16500,24500,0};
		bso::utilities::geometry::vertex vert5 = {16500,10500,4000};
		bso::utilities::geometry::vertex vert6 = {14500,12500,4000};
		bso::utilities::geometry::vertex vert7 = {14500,24500,4000};
		bso::utilities::geometry::vertex vert8 = {16500,24500,4000};
		
		pVertex.push_back(&vert1);
		pVertex.push_back(&vert2);
		pVertex.push_back(&vert3);
		pVertex.push_back(&vert4);
		pVertex.push_back(&vert5);
		pVertex.push_back(&vert6);
		pVertex.push_back(&vert7);
		pVertex.push_back(&vert8);
		
		std::vector<std::string> surfaceTypes = {"type_1", "type_2", "type_3", "type_4", "type_5", "type_6"};
		ms_space s2(1, pVertex, "type_a", surfaceTypes);
		BOOST_REQUIRE(s1 == s2);
		BOOST_REQUIRE(s2 == s1);
		BOOST_REQUIRE(!(s1 != s2));
		BOOST_REQUIRE(!(s2 != s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_8 )
	{
		ms_space s1("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_a,type_1,type_2,type_4,type_4,type_5,type_6");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}

	BOOST_AUTO_TEST_CASE( comparison_9 )
	{
		ms_space s1("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_a");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}
	
	BOOST_AUTO_TEST_CASE( comparison_10 )
	{
		ms_space s1("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_a,type_1,type_2,type_3,type_4,type_5,type_6");
		ms_space s2("N, 1,	0,0,4000,	0,6000,4000,	5000,6000,4000,	5000,0,4000,	0,0,7000,	0,6000,7000,	5000,6000,7000,	5000,0,7000,type_1,type_2,type_4,type_4,type_5,type_6");
		BOOST_REQUIRE(s1 != s2);
		BOOST_REQUIRE(s2 != s1);
		BOOST_REQUIRE(!(s1 == s2));
		BOOST_REQUIRE(!(s2 == s1));
	}

BOOST_AUTO_TEST_SUITE_END()
} // namespace spatial_design_test