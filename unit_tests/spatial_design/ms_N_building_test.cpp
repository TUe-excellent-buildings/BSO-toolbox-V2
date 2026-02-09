#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE ms_N_building
#endif

#include <bso/spatial_design/ms_N_building.hpp>

#include <vector>
#include <stdexcept>
#include <sstream>

#include <boost/test/included/unit_test.hpp>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace spatial_design_test {
using namespace bso::spatial_design;

BOOST_AUTO_TEST_SUITE( building_N_initialization )

	BOOST_AUTO_TEST_CASE( empty_initialization )
	{
		ms_N_building b1;
		std::stringstream sstr;
		sstr << b1;
		
		BOOST_REQUIRE(sstr.str() == "");
		BOOST_REQUIRE(b1.getSpacePtrs().size() == 0);
	}
	
	BOOST_AUTO_TEST_CASE( initialize_from_file_1 )
	{
		ms_N_building b1;

		BOOST_REQUIRE_THROW(b1 = ms_N_building("non_existent_file.txt"), std::invalid_argument);
	}

	BOOST_AUTO_TEST_CASE( initialize_from_file_2 )
	{
		ms_N_building b1("spatial_design/ms_N_test_5.txt");
		std::stringstream sstr;
		sstr << b1;
		
		ms_space* s1 = b1.getSpacePtrs()[0];
		
		std::vector<bso::utilities::geometry::vertex> pVertex  = s1->getVertices();
		
		bso::utilities::geometry::vertex p1 = {0,0,0};
		bso::utilities::geometry::vertex p2 = {6000,0,0};
		bso::utilities::geometry::vertex p3 = {0,6000,0};
		bso::utilities::geometry::vertex p4 = {6000,6000,0};
		bso::utilities::geometry::vertex p5 = {0,0,6000};
		bso::utilities::geometry::vertex p6 = {6000,0,6000};
		bso::utilities::geometry::vertex p7 = {0,6000,6000};
		bso::utilities::geometry::vertex p8 = {6000,6000,6000};	
		
		bool verticesCheck = true;
		if(std::find(pVertex.begin(),pVertex.end(),p1) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p2) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p3) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p4) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p5) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p6) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p7) == pVertex.end()){verticesCheck = false;}
		if(std::find(pVertex.begin(),pVertex.end(),p8) == pVertex.end()){verticesCheck = false;}																			   

	BOOST_REQUIRE(verticesCheck = true);
		BOOST_REQUIRE(sstr.str() == "N,1,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000");
		BOOST_REQUIRE(b1.getSpacePtrs().size() == 1);
	}

	BOOST_AUTO_TEST_CASE(getSpacePtrWithID_returns_correct_pointer_and_throws_on_missing)
{
    // Create two simple spaces using the "N,<id>,..." textual definition.
    // Coordinates are arbitrary but must form a valid 8-vertex definition for ms_space.
    std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
    std::string s2 = "N,2, 10,10,0, 11,10,0, 10,11,0, 11,11,0, 10,10,3, 11,10,3, 10,11,3, 11,11,3";

    ms_space sp1(s1);
    ms_space sp2(s2);

    ms_N_building b;
    // addSpace will make internal copies, preserving IDs from the ms_space objects
    b.addSpace(sp1);
    b.addSpace(sp2);

    // Retrieve pointers to internal spaces and verify ID and identity
    ms_space* p1 = b.getSpacePtrWithID(1);
    ms_space* p2 = b.getSpacePtrWithID(2);

    BOOST_REQUIRE(p1 != nullptr);
    BOOST_REQUIRE(p2 != nullptr);
    BOOST_CHECK_EQUAL(p1->getID(), 1u);
    BOOST_CHECK_EQUAL(p2->getID(), 2u);

    // Verify the returned pointers match one of the pointers in getSpacePtrs()
    auto vec = b.getSpacePtrs();
    bool found1 = false, found2 = false;
    for (auto ptr : vec) {
        if (ptr == p1) found1 = true;
        if (ptr == p2) found2 = true;
    }
    BOOST_CHECK(found1);
    BOOST_CHECK(found2);

    // Asking for a non-existing ID should throw std::runtime_error
    BOOST_CHECK_THROW(b.getSpacePtrWithID(9999), std::runtime_error);
}
	
	BOOST_AUTO_TEST_CASE( initialize_from_file_3 )
	{
		ms_N_building b1("spatial_design/ms_N_test_6.txt");
		
		std::stringstream sstr, check;
		sstr << b1;
		check << "N,1,	0,0,0,	4000,0,0,	4000,3000,0,	0,3000,0,	0,0,2000,	4000,0,2000,	4000,3000,2000,	0,3000,2000" << std::endl
					<< "N,2,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000" ;
		
		ms_space* s1 = b1.getSpacePtrs()[0];

		BOOST_REQUIRE(sstr.str() == check.str());
		BOOST_REQUIRE(b1.getSpacePtrs().size() == 2);
		
		sstr.str(std::string()); // clear the string stream
		ms_N_building b2 = b1; // copy contructor
		sstr << b2;

		BOOST_REQUIRE(sstr.str() == check.str());
		BOOST_REQUIRE(b2.getSpacePtrs().size() == 2);
	}
	
	BOOST_AUTO_TEST_CASE( non_valid_initialization_1 )
	{
		BOOST_REQUIRE_THROW(ms_N_building b5("spatial_design/wrong_ms_N_test_2.txt"), std::invalid_argument);
	}
	BOOST_AUTO_TEST_CASE( non_valid_initialization_2 )
	{
		BOOST_REQUIRE_THROW(ms_N_building b7("spatial_design/wrong_ms_N_test_3.txt"), std::invalid_argument);
	}
	
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( building_N_functions )

	BOOST_AUTO_TEST_CASE( getSpacePtr_1 )
	{
		ms_N_building b1("spatial_design/ms_N_test_5.txt");
		ms_space* s1 = b1.getSpacePtrs()[0];
		ms_space s2 = ms_space(*s1);
		
		BOOST_REQUIRE(s1 == b1.getSpacePtr(s2));
		b1.deleteSpace(s1);
		BOOST_REQUIRE_THROW(s1 == b1.getSpacePtr(s2), std::runtime_error);
	}
	
	BOOST_AUTO_TEST_CASE( getSpacePtr_2 )
	{ 
		ms_N_building b1;
		
		std::vector <bso::utilities::geometry::vertex> pVert;
		std::vector <bso::utilities::geometry::vertex> pVert1;
		std::vector <bso::utilities::geometry::vertex> pVert2;
		std::vector <bso::utilities::geometry::vertex> pVert3;

		// pVert space 0
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {1000,0,0};
		bso::utilities::geometry::vertex v3 = {1000,2000,0};
		bso::utilities::geometry::vertex v4 = {0,2000,0};
		bso::utilities::geometry::vertex v5 = {1000,0,3000};
		bso::utilities::geometry::vertex v6 = {1000,2000,3000};
		bso::utilities::geometry::vertex v7 = {1000,2000,3000};
		bso::utilities::geometry::vertex v8 = {0,2000,3000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		// pVert space 1
		bso::utilities::geometry::vertex v11 = {0,0,0};
		bso::utilities::geometry::vertex v21 = {1000,0,0};
		bso::utilities::geometry::vertex v31 = {1000,2000,0};
		bso::utilities::geometry::vertex v41 = {0,2000,0};
		bso::utilities::geometry::vertex v51 = {1000,0,3000};
		bso::utilities::geometry::vertex v61 = {1000,2000,3000};
		bso::utilities::geometry::vertex v71 = {1000,2000,3000};
		bso::utilities::geometry::vertex v81 = {0,2000,3000};
		pVert1.push_back(v11);
		pVert1.push_back(v21);
		pVert1.push_back(v31);
		pVert1.push_back(v41);
		pVert1.push_back(v51);
		pVert1.push_back(v61);
		pVert1.push_back(v71);
		pVert1.push_back(v81);

		// pVert space 2
		bso::utilities::geometry::vertex v12 = {4000,0,0};
		bso::utilities::geometry::vertex v22 = {5000,1000,0};
		bso::utilities::geometry::vertex v32 = {5000,3000,0};
		bso::utilities::geometry::vertex v42 = {4000,3000,0};
		bso::utilities::geometry::vertex v52 = {4000,0,3000};
		bso::utilities::geometry::vertex v62 = {5000,1000,3000};
		bso::utilities::geometry::vertex v72 = {5000,3000,3000};
		bso::utilities::geometry::vertex v82 = {4000,3000,3000};
		pVert2.push_back(v12);
		pVert2.push_back(v22);
		pVert2.push_back(v32);
		pVert2.push_back(v42);
		pVert2.push_back(v52);
		pVert2.push_back(v62);
		pVert2.push_back(v72);
		pVert2.push_back(v82);

		// pVert space 3
		bso::utilities::geometry::vertex v13 = {4000,0,0};
		bso::utilities::geometry::vertex v23 = {5000,1000,0};
		bso::utilities::geometry::vertex v33 = {5000,3000,0};
		bso::utilities::geometry::vertex v43 = {4000,3000,0};
		bso::utilities::geometry::vertex v53 = {4000,0,3000};
		bso::utilities::geometry::vertex v63 = {5000,1000,3000};
		bso::utilities::geometry::vertex v73 = {5000,3000,3000};
		bso::utilities::geometry::vertex v83 = {4000,3000,3000};
		pVert3.push_back(v13);
		pVert3.push_back(v23);
		pVert3.push_back(v33);
		pVert3.push_back(v43);
		pVert3.push_back(v53);
		pVert3.push_back(v63);
		pVert3.push_back(v73);
		pVert3.push_back(v83);

		// define spaces
		ms_space s1(1,pVert);
		ms_space s2(2,pVert1);
		ms_space s3(3,pVert2);
		ms_space s4(4,pVert3);
		
		std::vector<ms_space*> testPtrs;
		testPtrs.push_back(&s1);
		testPtrs.push_back(&s2);
		testPtrs.push_back(&s3);
		
		// add spaces
		b1.addSpace(s1);
		b1.addSpace(s2);
		b1.addSpace(s3);
		
		std::vector<ms_space*> addedPtrs = b1.getSpacePtrs();
		bool found[2] = {false};
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (*(addedPtrs[i]) == *(testPtrs[i]))
				{
					found[i] = true;
					break;
				}
			}
		}
		
		// Check requirements
		BOOST_REQUIRE(addedPtrs[0] == b1.getSpacePtr(s1));
		BOOST_REQUIRE(found[0] && found[1] && found[2]);
		BOOST_REQUIRE_THROW(b1.getSpacePtr(s4), std::runtime_error);
	}
	
	BOOST_AUTO_TEST_CASE( getLastSpaceID )
	{
		ms_N_building b1;
		BOOST_REQUIRE(b1.getLastSpaceID() == 0);
		
		std::vector <bso::utilities::geometry::vertex> pVert;
		
		bso::utilities::geometry::vertex v1 = {0,0,1000};
		bso::utilities::geometry::vertex v2 = {2000,0,1000};
		bso::utilities::geometry::vertex v3 = {0,3000,1000};
		bso::utilities::geometry::vertex v4 = {2000,2000,1000};
		bso::utilities::geometry::vertex v5 = {2000,0,2000};
		bso::utilities::geometry::vertex v6 = {0,3000,2000};
		bso::utilities::geometry::vertex v7 = {2000,2000,2000};
		bso::utilities::geometry::vertex v8 = {2000,0,2000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		b1.addSpace(ms_space(1,pVert));					   
		BOOST_REQUIRE(b1.getLastSpaceID() == 1);
		b1.addSpace(ms_space(131,pVert));
		BOOST_REQUIRE(b1.getLastSpaceID() == 131);
	}
	
	BOOST_AUTO_TEST_CASE( getinsertFileName )
	{
		ms_N_building b1("spatial_design/ms_N_test_1.txt");
		std::string textfileName = "spatial_design/ms_N_test_1.txt";
		BOOST_REQUIRE(b1.getInsertFileName() == textfileName);
	}
	
	BOOST_AUTO_TEST_CASE( getVolume_1 )
	{ 
		ms_N_building b1("spatial_design/ms_N_test_1.txt");
		
		BOOST_REQUIRE(b1.getVolume() == 6);
		
		b1.scale();
		BOOST_REQUIRE(round(b1.getVolume()) == 12);
	}

	BOOST_AUTO_TEST_CASE( getVolume_2 )
	{ 
		ms_N_building b1("spatial_design/ms_N_test_8.txt");
		BOOST_REQUIRE(b1.getVolume() == 4.5);
		
		b1.scale();

		BOOST_REQUIRE(round(b1.getVolume()) == 9);
	}

	BOOST_AUTO_TEST_CASE( getFloorArea_2 )
	{ 
		ms_N_building b1("spatial_design/ms_N_test_8.txt");
		BOOST_REQUIRE(b1.getFloorArea() == 1500000);
	}

	BOOST_AUTO_TEST_CASE(getNrElevations_basic_distinct_levels)
	{
		// space with bottom z=0 top z=2
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,2, 1,0,2, 0,1,2, 1,1,2";
		// space with bottom z=2 top z=4
		std::string s2 = "N,2, 0,0,2, 1,0,2, 0,1,2, 1,1,2, 0,0,4, 1,0,4, 0,1,4, 1,1,4";
		// space with bottom z=0 top z=4
		std::string s3 = "N,3, 1,0,0, 2,0,0, 1,1,0, 2,1,0, 1,0,4, 2,0,4, 1,1,4, 2,1,4";

		ms_space sp1(s1);
		ms_space sp2(s2);
		ms_space sp3(s3);

		ms_N_building b;
		b.addSpace(sp1);
		b.addSpace(sp2);
		b.addSpace(sp3);

		// Expect two distinct elevation levels detected as the roof is not an elevation of a building
		BOOST_CHECK_EQUAL(b.getNrElevations(), 2);
	}

	BOOST_AUTO_TEST_CASE(getNrElevations_tolerance_merging)
	{
		// base space center at 1.0
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,2, 1,0,2, 0,1,2, 1,1,2";
		// slightly shifted top such that center ~= 1.00025 (difference 0.00025)
		std::string s_shifted = "N,2, 1,0,0, 2,0,0, 1,1,0, 2,1,0, 1,0,2.0005, 2,0,2.0005, 1,1,2.0005, 2,1,2.0005";

		ms_space sp1(s1);
		ms_space sp2(s_shifted);

		ms_N_building b;
		b.addSpace(sp1);
		b.addSpace(sp2);

		// default tolerance in getNrElevations is 1e-3; centers differ by ~2.5e-4 -> considered same
		BOOST_CHECK_EQUAL(b.getNrElevations(), 1);
	}

	BOOST_AUTO_TEST_CASE(getAverageHorizontalRibSize_basic_cube)
	{
		// Define a unit cube (horizontal edge lengths = 1.0)
		std::string s = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		ms_space sp(s);

		ms_N_building b;
		b.addSpace(sp);

		// For a unit cube: bottom rectangle edges: two edges length 1, two edges length 1
		// top rectangle same -> total horizontal edges considered should average to 1.0
		double avg = b.getAverageHorizontalRibSize();
		BOOST_CHECK_CLOSE(avg, 1.0, 1e-6); // relative tolerance 1e-6
	}

	BOOST_AUTO_TEST_CASE(getAverageHorizontalRibSize_multiple_spaces_averages_correctly)
	{
		// First cube: 1x1 base -> horizontal ribs = 1.0
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		// Second rectangular prism: base 2 x 3 -> horizontal ribs lengths 2 and 3; average for that prism = (2+3+2+3+2+3+2+3)/8 = 2.5
		std::string s2 = "N,2, 0,0,0, 2,0,0, 0,3,0, 2,3,0, 0,0,1, 2,0,1, 0,3,1, 2,3,1";

		ms_space sp1(s1);
		ms_space sp2(s2);

		ms_N_building b;
		b.addSpace(sp1);
		b.addSpace(sp2);

		// Compute expected average across both spaces:
		// cube contributes avg 1.0 over 8 horizontal ribs -> sum = 8 * 1.0 = 8.0
		// prism contributes avg 2.5 over 8 horizontal ribs -> sum = 8 * 2.5 = 20.0
		// combined average = (8.0 + 20.0) / (8 + 8) = 28.0 / 16 = 1.75
		double expected = 1.75;
		double avg = b.getAverageHorizontalRibSize();
		BOOST_CHECK_CLOSE(avg, expected, 1e-6);
	}

	BOOST_AUTO_TEST_CASE(getSpaceVolumeRatio_basic)
	{
		// Unit cube -> volume = 1
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		// 2x2x2 cube -> volume = 8
		std::string s2 = "N,2, 0,0,0, 2,0,0, 0,2,0, 2,2,0, 0,0,2, 2,0,2, 0,2,2, 2,2,2";

		ms_space sp1(s1);
		ms_space sp2(s2);

		ms_N_building b;
		b.addSpace(sp1);
		b.addSpace(sp2);

		double expected = 1.0 / 8.0; // minVolume / maxVolume
		double ratio = b.getSpaceVolumeRatio();
		BOOST_CHECK_CLOSE(ratio, expected, 1e-6);
	}

	BOOST_AUTO_TEST_CASE(getSpaceVolumeRatio_throws_with_insufficient_spaces)
	{
		std::string s = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		ms_space sp(s);
		ms_N_building b;
		b.addSpace(sp);

		BOOST_CHECK_THROW(b.getSpaceVolumeRatio(), std::invalid_argument);
	}

	BOOST_AUTO_TEST_CASE(getSpaceFloorAreaRatio_basic)
	{
		// Unit square base -> floor area = 1
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		// 2x2 base -> floor area = 4
		std::string s2 = "N,2, 0,0,0, 2,0,0, 0,2,0, 2,2,0, 0,0,1, 2,0,1, 0,2,1, 2,2,1";

		ms_space sp1(s1);
		ms_space sp2(s2);

		ms_N_building b;
		b.addSpace(sp1);
		b.addSpace(sp2);

		double expected = 1.0 / 4.0; // minArea / maxArea
		double ratio = b.getSpaceFloorAreaRatio();
		BOOST_CHECK_CLOSE(ratio, expected, 1e-6);
	}

	BOOST_AUTO_TEST_CASE(getSpaceFloorAreaRatio_throws_with_insufficient_spaces)
	{
		std::string s = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		ms_space sp(s);
		ms_N_building b;
		b.addSpace(sp);

		BOOST_CHECK_THROW(b.getSpaceFloorAreaRatio(), std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE( setZZero_1 )
	{
		ms_N_building b1("spatial_design/ms_N_test_3.txt");
		b1.setZZero();
		std::stringstream sstr;
		sstr << b1;
		BOOST_REQUIRE(sstr.str() == "N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000\nN,2,	0,0,4303,	3000,0,4303,	0,3000,4303,	3000,3000,4303,	0,0,7303,	3000,0,7303,	0,3000,7303,	3000,3000,7303");
	}
	
	
	BOOST_AUTO_TEST_CASE( setZZero_2 )
	{ 
		ms_N_building b1("spatial_design/ms_N_test_4.txt");
		b1.setZZero();
		std::stringstream sstr;
		sstr << b1;
		BOOST_REQUIRE(sstr.str() == "N,1,	0,0,-697,	3000,0,-697,	0,3000,-697,	3000,3000,-697,	0,0,2303,	3000,0,2303,	0,3000,2303,	3000,3000,2303\nN,2,	0,0,5000,	3000,0,5000,	0,3000,5000,	3000,3000,5000,	0,0,8000,	3000,0,8000,	0,3000,8000,	3000,3000,8000");
	}
	
	
	BOOST_AUTO_TEST_CASE( addSpace_and_Comparison )
	{ 
		ms_N_building b1;
		ms_N_building b2("spatial_design/ms_N_test_3.txt");

		// space 1
		std::vector <bso::utilities::geometry::vertex> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {3000,0,0};
		bso::utilities::geometry::vertex v3 = {0,3000,0};
		bso::utilities::geometry::vertex v4 = {3000,3000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {3000,0,3000};
		bso::utilities::geometry::vertex v7 = {0,3000,3000};
		bso::utilities::geometry::vertex v8 = {3000,3000,3000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		// space 2
		std::vector <bso::utilities::geometry::vertex> pVert2;
		bso::utilities::geometry::vertex v12 = {0,0,4303};
		bso::utilities::geometry::vertex v22 = {3000,0,4303};
		bso::utilities::geometry::vertex v32 = {0,3000,4303};
		bso::utilities::geometry::vertex v42 = {3000,3000,4303};
		bso::utilities::geometry::vertex v52 = {0,0,7303};
		bso::utilities::geometry::vertex v62 = {3000,0,7303};
		bso::utilities::geometry::vertex v72 = {0,3000,7303};
		bso::utilities::geometry::vertex v82 = {3000,3000,7303};
		pVert2.push_back(v12);
		pVert2.push_back(v22);
		pVert2.push_back(v32);
		pVert2.push_back(v42);
		pVert2.push_back(v52);
		pVert2.push_back(v62);
		pVert2.push_back(v72);
		pVert2.push_back(v82);
		
		b1.addSpace(ms_space(1,pVert));
		b1.addSpace(ms_space(2,pVert2));

		BOOST_REQUIRE(b1 != b2);
		BOOST_REQUIRE(b2 != b1);

		b2.setZZero();
		
		// Check if setZZero() is functioning correctly
		BOOST_REQUIRE(b1 == b2);
		BOOST_REQUIRE(b2 == b1);
	}
	
	BOOST_AUTO_TEST_CASE( addSpace_and_Comparison_2 )
	{
		ms_N_building b1;
		ms_N_building b2;

		// space 1
		std::vector <bso::utilities::geometry::vertex> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {3000,0,0};
		bso::utilities::geometry::vertex v3 = {3000,3000,0};
		bso::utilities::geometry::vertex v4 = {0,3000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {3000,0,3000};
		bso::utilities::geometry::vertex v7 = {3000,3000,3000};
		bso::utilities::geometry::vertex v8 = {0,3000,3000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		// space 2; the same as space 1 however with different definition order of vertices
		std::vector <bso::utilities::geometry::vertex> pVert2;
		bso::utilities::geometry::vertex v12 = {3000,3000,3000};
		bso::utilities::geometry::vertex v22 = {3000,3000,0};
		bso::utilities::geometry::vertex v32 = {3000,0,0};
		bso::utilities::geometry::vertex v42 = {0,3000,0};
		bso::utilities::geometry::vertex v52 = {0,0,3000};
		bso::utilities::geometry::vertex v62 = {3000,0,3000};
		bso::utilities::geometry::vertex v72 = {0,0,0};
		bso::utilities::geometry::vertex v82 = {0,3000,3000};
		pVert2.push_back(v12);
		pVert2.push_back(v22);
		pVert2.push_back(v32);
		pVert2.push_back(v42);
		pVert2.push_back(v52);
		pVert2.push_back(v62);
		pVert2.push_back(v72);
		pVert2.push_back(v82);
		
		b1.addSpace(ms_space(1,pVert));
		b1.addSpace(ms_space(2,pVert2));

		b2.addSpace(ms_space(1,pVert));
		b2.addSpace(ms_space(3,pVert2));

		BOOST_REQUIRE(b1 != b2);
		BOOST_REQUIRE(b2 != b1);

		b2.getSpacePtrs().back()->setID(2);
		
		BOOST_REQUIRE(b1 == b2);
		BOOST_REQUIRE(b2 == b1);
	}

		BOOST_AUTO_TEST_CASE( addSpace_and_Comparison_3 )
	{ 
		ms_N_building b1;
		ms_N_building b2;
		ms_N_building b3;
		ms_N_building b4;

		// space 1
		std::vector <bso::utilities::geometry::vertex> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {3000,0,0};
		bso::utilities::geometry::vertex v3 = {0,3000,0};
		bso::utilities::geometry::vertex v4 = {3000,3000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {3000,0,3000};
		bso::utilities::geometry::vertex v7 = {0,3000,3000};
		bso::utilities::geometry::vertex v8 = {3000,3000,3000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		// space 2
		std::vector <bso::utilities::geometry::vertex> pVert2;
		bso::utilities::geometry::vertex v12 = {0,0,4303};
		bso::utilities::geometry::vertex v22 = {3000,0,4303};
		bso::utilities::geometry::vertex v32 = {0,3000,4303};
		bso::utilities::geometry::vertex v42 = {3000,3000,4303};
		bso::utilities::geometry::vertex v52 = {0,0,7303};
		bso::utilities::geometry::vertex v62 = {3000,0,7303};
		bso::utilities::geometry::vertex v72 = {0,3000,7303};
		bso::utilities::geometry::vertex v82 = {3000,3000,7303};
		pVert2.push_back(v12);
		pVert2.push_back(v22);
		pVert2.push_back(v32);
		pVert2.push_back(v42);
		pVert2.push_back(v52);
		pVert2.push_back(v62);
		pVert2.push_back(v72);
		pVert2.push_back(v82);

		// space 3; diferent order of vertices insertion
		std::vector <bso::utilities::geometry::vertex> pVert3;
		bso::utilities::geometry::vertex v13 = {0,3000,7303};
		bso::utilities::geometry::vertex v23 = {0,3000,4303};
		bso::utilities::geometry::vertex v33 = {3000,0,4303};
		bso::utilities::geometry::vertex v43 = {3000,3000,4303};
		bso::utilities::geometry::vertex v53 = {0,0,7303};
		bso::utilities::geometry::vertex v63 = {3000,0,7303};
		bso::utilities::geometry::vertex v73 = {0,0,4303};
		bso::utilities::geometry::vertex v83 = {3000,3000,7303};
		pVert3.push_back(v12);
		pVert3.push_back(v22);
		pVert3.push_back(v32);
		pVert3.push_back(v42);
		pVert3.push_back(v52);
		pVert3.push_back(v62);
		pVert3.push_back(v72);
		pVert3.push_back(v82);
		
		// base BSD to compare to
		b1.addSpace(ms_space(1,pVert));
		b1.addSpace(ms_space(2,pVert2));

		// Exactly same spaces but inserted in a diferent order
		b2.addSpace(ms_space(2,pVert2));
		b2.addSpace(ms_space(1,pVert));
		
		// Exactly the same spaces in geometry and properties but the space ID is diferent
		b3.addSpace(ms_space(2,pVert));
		b3.addSpace(ms_space(1,pVert2));

		// Exactly the same spaces in geometry but not in vertex order
		b4.addSpace(ms_space(1,pVert));
		b4.addSpace(ms_space(2,pVert3));

		BOOST_REQUIRE(b1 == b2);
		BOOST_REQUIRE(b2 == b1);
		BOOST_REQUIRE(b1 != b3);
		BOOST_REQUIRE(b1 == b4);
	}
	
	BOOST_AUTO_TEST_CASE( deleteSpace )
	{
		ms_N_building b1("spatial_design/ms_N_test_1.txt");
		ms_N_building b2;
		
		// space 1
		std::vector <bso::utilities::geometry::vertex> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {1000,0,0};
		bso::utilities::geometry::vertex v3 = {1000,2000,0};
		bso::utilities::geometry::vertex v4 = {0,2000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {1000,0,3000};
		bso::utilities::geometry::vertex v7 = {1000,2000,3000};
		bso::utilities::geometry::vertex v8 = {0,2000,3000};
		pVert.push_back(v1);
		pVert.push_back(v2);
		pVert.push_back(v3);
		pVert.push_back(v4);
		pVert.push_back(v5);
		pVert.push_back(v6);
		pVert.push_back(v7);
		pVert.push_back(v8);

		// space 2
		std::vector <bso::utilities::geometry::vertex> pVert2;
		bso::utilities::geometry::vertex v12 = {1000,0,0};
		bso::utilities::geometry::vertex v22 = {4000,0,0};
		bso::utilities::geometry::vertex v32 = {1000,4000,0};
		bso::utilities::geometry::vertex v42 = {4000,4000,0};
		bso::utilities::geometry::vertex v52 = {1000,0,3000};
		bso::utilities::geometry::vertex v62 = {4000,0,3000};
		bso::utilities::geometry::vertex v72 = {1000,4000,3000};
		bso::utilities::geometry::vertex v82 = {4000,4000,3000};
		pVert2.push_back(v12);
		pVert2.push_back(v22);
		pVert2.push_back(v32);
		pVert2.push_back(v42);
		pVert2.push_back(v52);
		pVert2.push_back(v62);
		pVert2.push_back(v72);
		pVert2.push_back(v82);

		// space 3
		std::vector <bso::utilities::geometry::vertex> pVert3;
		bso::utilities::geometry::vertex v13 = {0,0,4303};
		bso::utilities::geometry::vertex v23 = {3000,0,4303};
		bso::utilities::geometry::vertex v33 = {0,3000,4303};
		bso::utilities::geometry::vertex v43 = {3000,3000,4303};
		bso::utilities::geometry::vertex v53 = {3000,0,7303};
		bso::utilities::geometry::vertex v63 = {0,3000,7303};
		bso::utilities::geometry::vertex v73 = {3000,3000,7303};
		bso::utilities::geometry::vertex v83 = {0,0,7303};
		pVert3.push_back(v13);
		pVert3.push_back(v23);
		pVert3.push_back(v33);
		pVert3.push_back(v43);
		pVert3.push_back(v53);
		pVert3.push_back(v63);
		pVert3.push_back(v73);
		pVert3.push_back(v83);


		ms_space s1(1,pVert);
		ms_space s2(2,pVert2);
		ms_space s3(3,pVert3);
		
		b2.addSpace(s1);
		b2.addSpace(s2);
		b2.addSpace(s3);
		
		b2.deleteSpace(s2);
		b2.deleteSpace(s3);
		BOOST_REQUIRE(b1 == b2);
		
		b2.deleteSpace(&s1);
		BOOST_REQUIRE(b2.getSpacePtrs().empty());
		BOOST_REQUIRE_THROW(b2.deleteSpace(&s2), std::runtime_error);
		BOOST_REQUIRE_THROW(b2.deleteSpace(s1), std::runtime_error);
	}
	
	BOOST_AUTO_TEST_CASE( scale_1 )
	{ // test of an orthogonal design for scaling
		ms_N_building b1("spatial_design/ms_N_test_3.txt");

		b1.scale({{0,1.5},{1,2},{2,1.23}});

		ms_space space1("N,1,	0,0,857.30999999999995,	4500,0,857.30999999999995,	0,6000,857.30999999999995,	4500,6000,857.30999999999995,   0,0,4547.3099999999995, 4500,0,4547.3099999999995,	0,6000,4547.3099999999995,	4500,6000,4547.3099999999995");
		ms_space space2("N,2,    0,0,6150,       4500,0,6150,    0,6000,6150,    4500,6000,6150, 0,0,9840,       4500,0,9840,    0,6000,9840,    4500,6000,9840");

		BOOST_REQUIRE(b1.getSpacePtrs().size() == 2);
		
		int count = 0;
		for(auto& sp : b1)
		{
			if (count == 0)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space1.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			else if (count == 1)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space2.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			count++;
		}

		b1.scale();
		
		ms_space space3("N,1,      0,0,857.30999999999995, 6363.9610306789282,0,857.30999999999995,        0,8485.2813742385715,857.30999999999995,        6363.9610306789282,8485.2813742385715,857.30999999999995,       0,0,4547.3099999999995, 6363.9610306789282,0,4547.3099999999995,        0,8485.2813742385715,4547.3099999999995,        6363.9610306789282,8485.2813742385715,4547.3099999999995");
		ms_space space4("N,2,    0,0,6150,       6363.9610306789282,0,6150,      0,8485.2813742385715,6150,      6363.9610306789282,8485.2813742385715,6150,     0,0,9840,       6363.9610306789282,0,9840,      0,8485.2813742385715,9840,      6363.9610306789282,8485.2813742385715,9840");

		count = 0;
		for(auto& sp : b1)
		{
			if (count == 0)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space3.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			else if (count == 1)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space4.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			count++;
		}
		
		BOOST_REQUIRE_THROW(b1.scale({{3,1.5}}), std::invalid_argument);
		BOOST_REQUIRE_THROW(b1.scale({{1,1.0},{1,1.0}}), std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE( scale_2 )
	{ // test of an non-orthogonal design for scaling
		ms_N_building b1("spatial_design/ms_N_test_9.txt");
		
		b1.scale({{0,1.5},{1,2},{2,1.98}});

		ms_space space1("N,1,	0,0,0,	4500,0,0,	0,8000,0,	6000,8000,0,	0,0,5940,	4500,0,5940,	0,8000,5940,	6000,8000,5940");
		ms_space space2("N,2,	0,0,5940,	4500,-4000,5940,	0,6000,5940,	4500,6000,5940,	0,0,15840,	4500,-4000,15840,	0,6000,15840,	4500,6000,15840");

		BOOST_REQUIRE(b1.getSpacePtrs().size() == 2);
		
		int count = 0;
		for(auto& sp : b1)
		{
			if (count == 0)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space1.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			else if (count == 1)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space2.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			count++;
		}

		b1.scale();

		ms_space space3("N,1,       0,0,0,  6363.9610306789282,0,0, 0,11313.708498984761,0, 8485.2813742385715,11313.708498984761,0,        0,0,5940,       6363.9610306789282,0,5940,      0,11313.708498984761,5940,      8485.2813742385715,11313.708498984761,5940");
		ms_space space4("N,2,    0,0,5940,       6363.9610306789282,-5656.8542494923804,5940,    0,8485.2813742385715,5940,      6363.9610306789282,8485.2813742385715,5940,     0,0,15840,      6363.9610306789282,-5656.8542494923804,15840,   0,8485.2813742385715,15840,   6363.9610306789282,8485.2813742385715,15840");

		BOOST_REQUIRE(b1.getSpacePtrs().size() == 2);
		
		count = 0;
		for(auto& sp : b1)
		{
			if (count == 0)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space3.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			else if (count == 1)
			{
				for(const auto& v : sp->getVertices())
				{
					bool isFound = false;
					for(auto& v_ref : space4.getVertices())
					{
						if (v->isSameAs(*v_ref, 0.0000001))
						{
							isFound = true;
							break;
						}
					}
					BOOST_REQUIRE(isFound);
				}
			}
			count++;
		}
	}
	
	BOOST_AUTO_TEST_CASE( splitSpace_1a )
	{ // Base case for rectangular BSD
		ms_N_building b1("spatial_design/ms_N_test_1.txt");
		std::stringstream sstr;
		std::stringstream check;
		
		ms_space* spacePtr = b1.getSpacePtrs()[0];
		
		b1.splitSpace(spacePtr,{{1,2},{2,3}});
		sstr << b1;

		check << "N,2,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000" << std::endl
					<< "N,3,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000,	0,0,2000,	1000,0,2000,	0,1000,2000,	1000,1000,2000" << std::endl
					<< "N,4,	0,0,2000,	1000,0,2000,	0,1000,2000,	1000,1000,2000,	0,0,3000,	1000,0,3000,	0,1000,3000,	1000,1000,3000" << std::endl
					<< "N,5,	0,1000,0,	1000,1000,0,	0,2000,0,	1000,2000,0,	0,1000,1000,	1000,1000,1000,	0,2000,1000,	1000,2000,1000" << std::endl
					<< "N,6,	0,1000,1000,	1000,1000,1000,	0,2000,1000,	1000,2000,1000,	0,1000,2000,	1000,1000,2000,	0,2000,2000,	1000,2000,2000" << std::endl
					<< "N,7,	0,1000,2000,	1000,1000,2000,	0,2000,2000,	1000,2000,2000,	0,1000,3000,	1000,1000,3000,	0,2000,3000,	1000,2000,3000";
		BOOST_REQUIRE(sstr.str() == check.str());

		BOOST_REQUIRE_THROW(b1.splitSpace(b1.getSpacePtrs()[0],{{3,2},{2,3}}), std::invalid_argument); // non-existent axis
		BOOST_REQUIRE_THROW(b1.splitSpace(b1.getSpacePtrs()[0],{{1,2},{1,3}}), std::invalid_argument); // splitting twice over same axis
		BOOST_REQUIRE_THROW(b1.splitSpace(b1.getSpacePtrs()[0],{{1,1},{2,0}}), std::invalid_argument); // splitting space over an axis into less than two new spaces
		BOOST_REQUIRE_THROW(b1.getSpacePtr(spacePtr), std::runtime_error); // spacePtr is deleted after the split
		BOOST_REQUIRE_THROW(b1.splitSpace(spacePtr,{{1,2},{2,3}}), std::runtime_error); // spacePtr does not belong to b1 anymore and cannot be split
	}

	BOOST_AUTO_TEST_CASE( splitSpace_1b )
	{ // First used test case while developing the first split function for NOBSD
		bso::spatial_design::ms_N_building m1;
		m1.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0,	1000,0,0,	0,2000,0,	2000,2000,0,	0,0,3000,	1000,0,3000,	0,2000,3000,		2000,2000,3000,	"));
		std::stringstream sstr;
		std::stringstream check;
		
		bso::spatial_design::ms_space* spacePtr = m1.getSpacePtrs()[0];
		
		m1.splitSpace(spacePtr,{{1,2},{2,3}});
		sstr << m1;

		check << "N,2,	0,0,0,	1000,0,0,	0,1000,0,	1500,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1500,1000,1000" << std::endl
					<< "N,3,	0,0,1000,	1000,0,1000,	0,1000,1000,	1500,1000,1000,	0,0,2000,	1000,0,2000,	0,1000,2000,	1500,1000,2000" << std::endl
					<< "N,4,	0,0,2000,	1000,0,2000,	0,1000,2000,	1500,1000,2000,	0,0,3000,	1000,0,3000,	0,1000,3000,	1500,1000,3000" << std::endl
					<< "N,5,	0,1000,0,	1500,1000,0,	0,2000,0,	2000,2000,0,	0,1000,1000,	1500,1000,1000,	0,2000,1000,	2000,2000,1000" << std::endl
					<< "N,6,	0,1000,1000,	1500,1000,1000,	0,2000,1000,	2000,2000,1000,	0,1000,2000,	1500,1000,2000,	0,2000,2000,	2000,2000,2000" << std::endl
					<< "N,7,	0,1000,2000,	1500,1000,2000,	0,2000,2000,	2000,2000,2000,	0,1000,3000,	1500,1000,3000,	0,2000,3000,	2000,2000,3000";
		BOOST_REQUIRE(sstr.str() == check.str());
	}

	BOOST_AUTO_TEST_CASE( splitSpace_2 )
	{ // check a square space at a 45 degree angle to the axis (1,0,0); specifically, if it can find the axis to consider
		ms_N_building m1;
		m1.addSpace(ms_space("N, 1 ,	1000,0,0,	2000,1000,0,	1000,2000,0,	0,1000,0,	1000,0,1000,	2000,1000,1000,	1000,2000,1000,	0,1000,1000"));
		std::stringstream sstr;
		std::stringstream check;
		
		ms_space* spacePtr = m1.getSpacePtrs()[0];
		
		m1.splitSpace(spacePtr,{{0,2},{1,2}});
		sstr << m1;
		check << "N,2,	1000,0,0,	1500,500,0,	500,500,0,	1000,1000,0,	1000,0,1000,	1500,500,1000,	500,500,1000,	1000,1000,1000" << std::endl
					<< "N,3,	500,500,0,	1000,1000,0,	0,1000,0,	500,1500,0,	500,500,1000,	1000,1000,1000,	0,1000,1000,	500,1500,1000" << std::endl
					<< "N,4,	1500,500,0,	2000,1000,0,	1000,1000,0,	1500,1500,0,	1500,500,1000,	2000,1000,1000,	1000,1000,1000,	1500,1500,1000" << std::endl
					<< "N,5,	1000,1000,0,	1500,1500,0,	500,1500,0,	1000,2000,0,	1000,1000,1000,	1500,1500,1000,	500,1500,1000,	1000,2000,1000" ;
		BOOST_REQUIRE(sstr.str() == check.str());
	}

	BOOST_AUTO_TEST_CASE( splitSpace_3 )
	{ // check a space between axis 0,1,0 and 1,1,0; specifically, if it can find the axis to consider
		ms_N_building m1;
		m1.addSpace(ms_space("N, 1 ,	0,0,0,	1500,2000,0,	500,1500,0,	2000,3000,0,	0,0,2000,	1500,2000,2000,	500,1500,2000,	2000,3000,2000"));
		std::stringstream sstr;
		std::stringstream check;
		
		ms_space* spacePtr = m1.getSpacePtrs()[0];
		
		m1.splitSpace(spacePtr,{{2,2}});
		sstr << m1;

		check << "N,2,	500,1500,0,	2000,3000,0,	0,0,0,	1500,2000,0,	500,1500,1000,	2000,3000,1000,	0,0,1000,	1500,2000,1000" << std::endl
			  		<< "N,3,	500,1500,1000,	2000,3000,1000,	0,0,1000,	1500,2000,1000,	500,1500,2000,	2000,3000,2000,	0,0,2000,	1500,2000,2000";
		BOOST_REQUIRE(sstr.str() == check.str());
	}

	BOOST_AUTO_TEST_CASE( splitSpace_4 )
	{ // Check a space with lines over and behind the axis-x and -y
		ms_N_building m1;
		m1.addSpace(ms_space("N, 1 ,	1000,1000,0,	-1000,0,0,	500,-1000,0,	-1000,2000,0,	1000,1000,2000,	-1000,0,2000,	500,-1000,2000,	-1000,2000,2000"));
		std::stringstream sstr;
		std::stringstream check;
		
		ms_space* spacePtr = m1.getSpacePtrs()[0];
		
		m1.splitSpace(spacePtr,{{0,2},{1,4}});
		sstr << m1;

		check << "N,2,	-1000,2000,0,	0,1500,0,	-1000,1500,0,	-62.5,1000,0,	-1000,2000,2000,	0,1500,2000,	-1000,1500,2000,	-62.5,1000,2000" << std::endl
			  		<< "N,3,	-1000,1500,0,	-62.5,1000,0,	-1000,1000,0,	-125,500,0,	-1000,1500,2000,	-62.5,1000,2000,	-1000,1000,2000,	-125,500,2000" << std::endl
					<< "N,4,	-1000,1000,0,	-125,500,0,	-1000,500,0,	-187.5,0,0,	-1000,1000,2000,	-125,500,2000,	-1000,500,2000,	-187.5,0,2000" << std::endl
					<< "N,5,	-1000,500,0,	-187.5,0,0,	-1000,0,0,	-250,-500,0,	-1000,500,2000,	-187.5,0,2000,	-1000,0,2000,	-250,-500,2000" << std::endl
					<< "N,6,	0,1500,0,	1000,1000,0,	-62.5,1000,0,	875,500,0,	0,1500,2000,	1000,1000,2000,	-62.5,1000,2000,	875,500,2000" << std::endl
					<< "N,7,	-62.5,1000,0,	875,500,0,	-125,500,0,	750,0,0,	-62.5,1000,2000,	875,500,2000,	-125,500,2000,	750,0,2000" << std::endl
					<< "N,8,	-125,500,0,	750,0,0,	-187.5,0,0,	625,-500,0,	-125,500,2000,	750,0,2000,	-187.5,0,2000,	625,-500,2000" << std::endl
					<< "N,9,	-187.5,0,0,	625,-500,0,	-250,-500,0,	500,-1000,0,	-187.5,0,2000,	625,-500,2000,	-250,-500,2000,	500,-1000,2000" ;

		BOOST_REQUIRE(sstr.str() == check.str());
	}

	BOOST_AUTO_TEST_CASE( duplicateElevation_1 )
	{
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		
		ms1.duplicateElevation(1000,1000);
		BOOST_REQUIRE(ms1.getSpacePtrs().size()==2);

		std::vector<ms_space> checkSpaces;
		checkSpaces.push_back(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		checkSpaces.push_back(ms_space("N,2,	0,0,1000,	0,0,2000,	1000,0,1000,	1000,0,2000,	1000,1000,1000,	1000,1000,2000,	0,1000,1000,	0,1000,2000"));
		
		bool checked = true;
		int i=0;
		for(const auto j: ms1)
		{
			if(checkSpaces[i] != *j){checked = false;}
			i++;
		}

		BOOST_REQUIRE(checked);
	}

	BOOST_AUTO_TEST_CASE( duplicateElevation_2 )
	{
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		
		ms1.duplicateElevation(500,2000);
		BOOST_REQUIRE(ms1.getSpacePtrs().size()==2);

		std::vector<ms_space> checkSpaces;
		checkSpaces.push_back(ms_space("N,1,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		checkSpaces.push_back(ms_space("N,2,	0,0,1000,	0,0,3000,	1000,0,1000,	1000,0,3000,	1000,1000,1000,	1000,1000,3000,	0,1000,1000,	0,1000,3000"));
		
		bool checked = true;
		int i=0;
		for(const auto j: ms1)
		{
			if(checkSpaces[i] != *j){checked = false;}
			i++;
		}

		BOOST_REQUIRE(checked);
	}

	BOOST_AUTO_TEST_CASE( duplicateElevation_3 )
	{
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms_N_building ms2;
		ms2.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		
		ms1.duplicateElevation(0,2000);
		BOOST_REQUIRE(ms1.getSpacePtrs().size()==2);
		ms2.duplicateElevation(0,2000,false);
		BOOST_REQUIRE(ms2.getSpacePtrs().size()==2);

		std::vector<ms_space> checkSpaces;
		checkSpaces.push_back(ms_space("N, 1 ,	0,0,2000,	1000,0,2000,	0,1000,2000,	1000,1000,2000,	0,0,3000,	1000,0,3000,	0,1000,3000,	1000,1000,3000"));
		checkSpaces.push_back(ms_space("N,2,	0,0,0,	0,0,2000,	1000,0,0,	1000,0,2000,	1000,1000,0,	1000,1000,2000,	0,1000,0,	0,1000,2000"));
		
		bool checked1 = true;
		bool checked2 = true;
		int i=0;
		for(const auto j: ms1)
		{
			if(checkSpaces[i] != *j){checked1 = false;}
			i++;
		}
		i=0;
		for(const auto j: ms2)
		{
			if(checkSpaces[i] != *j){checked2 = false;}
			i++;
		}

		BOOST_REQUIRE(checked1);
		BOOST_REQUIRE(checked2);
	}

	BOOST_AUTO_TEST_CASE( duplicateElevation_4 )
	{
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms1.addSpace(ms_space("N, 2 ,	500,0,1000,	1000,0,1000,	500,1000,1000,	1000,1000,1000,	500,0,2000,	1000,0,2000,	500,1000,2000,	1000,1000,2000"));
		ms_N_building ms2(ms1);
		
		ms1.duplicateElevation(1000,2000,true);
		BOOST_REQUIRE(ms1.getSpacePtrs().size()==3);
		ms2.duplicateElevation(1000,2000,false);
		BOOST_REQUIRE(ms2.getSpacePtrs().size()==3);

		std::vector<ms_space> checkSpaces1;
		checkSpaces1.push_back(ms_space("N,1,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		checkSpaces1.push_back(ms_space("N,2,	500,0,3000,	1000,0,3000,	500,1000,3000,	1000,1000,3000,	500,0,4000,	1000,0,4000,	500,1000,4000,	1000,1000,4000"));
		checkSpaces1.push_back(ms_space("N,3,	0,0,1000,	0,0,3000,	1000,0,1000,	1000,0,3000,	1000,1000,1000,	1000,1000,3000,	0,1000,1000,	0,1000,3000"));

		std::vector<ms_space> checkSpaces2;
		checkSpaces2.push_back(ms_space("N,1,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		checkSpaces2.push_back(ms_space("N,2,	500,0,3000,	1000,0,3000,	500,1000,3000,	1000,1000,3000,	500,0,4000,	1000,0,4000,	500,1000,4000,	1000,1000,4000"));
		checkSpaces2.push_back(ms_space("N,3,	500,0,1000,	500,0,3000,	1000,0,1000,	1000,0,3000,	1000,1000,1000,	1000,1000,3000,	500,1000,1000,	500,1000,3000"));
		
		bool checked1 = true;
		bool checked2 = true;
		int i=0;
		for(const auto j: ms1)
		{
			if(checkSpaces1[i] != *j){checked1 = false;}
			i++;
		}
		i=0;
		for(const auto j: ms2)
		{
			if(checkSpaces2[i] != *j){checked2 = false;}
			i++;
		}

		BOOST_REQUIRE(checked1);
		BOOST_REQUIRE(checked2);
	}
	
	BOOST_AUTO_TEST_CASE( has_floating_spaces )
	{
		std::vector<ms_space*> floatingSpaces;
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms1.addSpace(ms_space("N, 2 ,	0,0,1500,	1000,0,1500,	0,1000,1500,	1000,1000,1500,	0,0,2500,	1000,0,2500,	0,1000,2500,	1000,1000,2500"));
		BOOST_REQUIRE(ms1.hasFloatingSpaces(floatingSpaces));

		floatingSpaces.clear();
		ms_N_building ms2;
		ms2.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms2.addSpace(ms_space("N, 2 ,	1000,1000,1000,	2000,1000,1000,	1000,2000,1000,	2000,2000,1000,	1000,1000,2000,	2000,1000,2000,	1000,2000,2000,	2000,2000,2000"));
		BOOST_REQUIRE(!ms2.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms3;
		ms3.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms3.addSpace(ms_space("N, 1 ,	1000,0,1000,	2000,0,1000,	0,1000,1000,	2000,1000,1000,	1000,0,2000,	2000,0,2000,	0,1000,2000,	2000,1000,2000"));
		BOOST_REQUIRE(!ms3.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms4;
		ms4.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms4.addSpace(ms_space("N, 1 ,	1000,500,500,	2000,500,500,	1000,1500,500,	2000,1500,500,	1000,500,1500,	2000,500,1500,	1000,1500,1500,	2000,1500,1500"));
		BOOST_REQUIRE(!ms4.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms5;
		ms5.addSpace(ms_space("N, 1 ,	0,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms5.addSpace(ms_space("N, 1 ,	0,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000,	0,0,2000,	1000,0,2000,	0,1000,2000,	1000,1000,2000"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));
	}

	BOOST_AUTO_TEST_CASE( has_floating_spaces_non_orthogonal )
	{ // a non-orthogonal version of the previous check
		std::vector<ms_space*> floatingSpaces;
		
		// Previous cases represented in non-orthogonal notation:
		ms_N_building ms1;
		ms1.addSpace(ms_space("N, 1 ,	0,0,0,	2000,0,0,	0,1000,0,	1000,1500,0,	0,0,1000,	2000,0,1000,	0,1000,1000,	1000,1500,1000"));
		ms1.addSpace(ms_space("N, 2 ,	0,0,1500,	1000,0,1500,	0,1500,1500,	1000,1000,1500,	0,0,2500,	1000,0,2500,	0,1500,2500,	1000,1000,2500"));
		BOOST_REQUIRE(ms1.hasFloatingSpaces(floatingSpaces));

		floatingSpaces.clear();
		ms_N_building ms2;
		ms2.addSpace(ms_space("N, 1 ,	0,0,0,	1500,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1500,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms2.addSpace(ms_space("N, 2 ,	1000,1000,1000,	2000,100,1000,	1000,2000,1000,	2000,2500,1000,	1000,1000,2000,	2000,100,2000,	1000,2000,2000,	2000,2500,2000"));
		BOOST_REQUIRE(!ms2.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms3;
		ms3.addSpace(ms_space("N, 1 ,	66.87,0,0,	1000,0,0,	0,1000,0,	1000,1000,0,	66.87,0,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms3.addSpace(ms_space("N, 2 ,	1000,0,1000,	3000,0,1000,	1000,1000,1000,	2000,1000,1000,	1000,0,2000,	3000,0,2000,	1000,1000,2000,	2000,1000,2000"));
		BOOST_REQUIRE(!ms3.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms4;
		ms4.addSpace(ms_space("N, 1 ,	0,500,0,	1000,0,0,	0,1000,0,	1000,1000,0,	0,500,1000,	1000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms4.addSpace(ms_space("N, 2 ,	1000,500,500,	2200,500,500,	1000,1500,500,	2000,1500,500,	1000,500,1500,	2200,500,1500,	1000,1500,1500,	2000,1500,1500"));
		BOOST_REQUIRE(!ms4.hasFloatingSpaces(floatingSpaces));
		
		floatingSpaces.clear();
		ms_N_building ms5;
		ms5.addSpace(ms_space("N, 1 ,	0,0,0,	1200,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	1200,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms5.addSpace(ms_space("N, 2 ,	0,0,1000,	1200,0,1000,	0,1000,1000,	1000,1000,1000,	0,0,2000,	1200,0,2000,	0,1000,2000,	1000,1000,2000"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));

		// uniquely non-orthogonal cases:
		floatingSpaces.clear();
		ms_N_building ms6;
		ms6.addSpace(ms_space("N, 1 ,	0,0,0,	2000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	2000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms6.addSpace(ms_space("N, 2 ,	1500,500,1000,	2500,500,1000,	1500,1000,1000,	2000,1000,1000,	1500,500,2000,	2500,500,2000,	1500,1000,2000,	2000,1000,2000"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));

		floatingSpaces.clear();
		ms_N_building ms7;
		ms7.addSpace(ms_space("N, 1 ,	0,0,0,	2000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	2000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms7.addSpace(ms_space("N, 2 ,	1500,500,500,	2500,500,500,	1500,1000,500,	2000,1000,500,	1500,500,1500,	2500,500,1500,	1500,1000,1500,	2000,1000,1500"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));

		floatingSpaces.clear();
		ms_N_building ms8;
		ms8.addSpace(ms_space("N, 1 ,	0,0,0,	2000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	2000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms8.addSpace(ms_space("N, 2 ,	1500,500,500,	2500,500,500,	1500,1000,500,	2000,1000,500,	1500,500,1000,	2500,500,1000,	1500,1000,1000,	2000,1000,1000"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));

		floatingSpaces.clear();
		ms_N_building ms9;
		ms9.addSpace(ms_space("N, 1 ,	0,0,0,	2000,0,0,	0,1000,0,	1000,1000,0,	0,0,1000,	2000,0,1000,	0,1000,1000,	1000,1000,1000"));
		ms9.addSpace(ms_space("N, 2 ,	1500,500,1000,	2500,500,1000,	1000,1000,1000,	2000,1000,1000,	1500,500,2000,	2500,500,2000,	1000,1000,2000,	2000,1000,2000"));
		BOOST_REQUIRE(!ms5.hasFloatingSpaces(floatingSpaces));
	}

	BOOST_AUTO_TEST_CASE( isFragmented )
	{
		// Two cubes sharing a face (connected)
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		std::string s2 = "N,2, 1,0,0, 2,0,0, 1,1,0, 2,1,0, 1,0,1, 2,0,1, 1,1,1, 2,1,1";
		ms_space sp1(s1);
		ms_space sp2(s2);
		ms_N_building b;
		std::vector<ms_space*> nonBiggest;
		BOOST_REQUIRE(!b.isFragmented(nonBiggest)); // empty building is not fragmented
		b.addSpace(sp1);
		nonBiggest.clear();
		BOOST_REQUIRE(!b.isFragmented(nonBiggest)); // single space is not fragmented
		b.addSpace(sp2);
		nonBiggest.clear();
		bool fragmented = b.isFragmented(nonBiggest);

		BOOST_CHECK_EQUAL(fragmented, false);
		BOOST_CHECK(nonBiggest.empty());

		// Add isolated cube at x = 10..11 (non-connected)
    	std::string s3 = "N,3, 10,0,0, 11,0,0, 10,1,0, 11,1,0, 10,0,1, 11,0,1, 10,1,1, 11,1,1";
		ms_space sp3(s3);
		b.addSpace(sp3);
		nonBiggest.clear();
		fragmented = b.isFragmented(nonBiggest);
		BOOST_CHECK_EQUAL(fragmented, true);
		BOOST_CHECK(nonBiggest.size() == 1);

		// Add additional isolated cube at x = 20..21 (non-connected)
    	std::string sAd = "N,4, 20,0,0, 21,0,0, 20,1,0, 21,1,0, 20,0,1, 21,0,1, 20,1,1, 21,1,1";
		ms_space spAd(sAd);
		b.addSpace(spAd);
		nonBiggest.clear();
		fragmented = b.isFragmented(nonBiggest);
		BOOST_CHECK_EQUAL(fragmented, true);
		BOOST_CHECK(nonBiggest.size() == 2);

		// Two cubes sharing a rib (connected)
		ms_N_building b2;
		b2.addSpace(sp1);
		std::string s4 = "N,4, 1,1,0, 2,1,0, 1,2,0, 2,2,0, 1,1,1, 2,1,1, 1,2,1, 2,2,1";
		ms_space sp4(s4);
		b2.addSpace(sp4);
		nonBiggest.clear();
		fragmented = b2.isFragmented(nonBiggest);
		BOOST_CHECK_EQUAL(fragmented, false);
		BOOST_CHECK(nonBiggest.empty());

		// Two cubes sharing a vertex (connected)
		ms_N_building b3;
		b3.addSpace(sp1);
		std::string s5 = "N,5, 1,1,1, 2,1,1, 1,2,1, 2,2,1, 1,1,2, 2,1,2, 1,2,2, 2,2,2";
		ms_space sp5(s5);
		b3.addSpace(sp5);
		nonBiggest.clear();
		fragmented = b3.isFragmented(nonBiggest);
		BOOST_CHECK_EQUAL(fragmented, false);
		BOOST_CHECK(nonBiggest.empty());
	}
		
	BOOST_AUTO_TEST_CASE( getNOBSDAtGround_1 ) 
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,0,3000,	1500,0,3000,	-1500,3000,3000,	1500,3000,3000,	-1500,0,6000,	1500,0,6000,	-1500,3000,6000,	1500,3000,6000");

		ms.addSpace(s1);
		ms.addSpace(s2);
		ms.addSpace(s3);

		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV;
		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV2;
		std::vector<double> allLevels;

		// These functions get the above defined information
		ms.getNOBSDAtGround(spaceAtGroundV, allLevels);
		ms.getNOBSDAtGround(spaceAtGroundV2);

		BOOST_REQUIRE(spaceAtGroundV.size() == 3);
		BOOST_REQUIRE(allLevels.size() == 3);
		BOOST_REQUIRE(allLevels[0] == 0);
		BOOST_REQUIRE(allLevels[1] == 3000);
		BOOST_REQUIRE(allLevels[2] == 6000);
		BOOST_REQUIRE(*(spaceAtGroundV[0].first) == s1);
		BOOST_REQUIRE(*(spaceAtGroundV[1].first) == s2);
		BOOST_REQUIRE(*(spaceAtGroundV[2].first) == s3);

		BOOST_REQUIRE(spaceAtGroundV2.size() == 3);
		BOOST_REQUIRE(*(spaceAtGroundV2[0].first) == s1);
		BOOST_REQUIRE(*(spaceAtGroundV2[1].first) == s2);
		BOOST_REQUIRE(*(spaceAtGroundV2[2].first) == s3);

		bso::utilities::geometry::quad_hexahedron QH1 = {{0,0,0},{3000,0,0,},{0,3000,0},{3000,3000,0},
														 {0,0,3000},{3000,0,3000},{0,3000,3000},{3000,3000,3000}};
		bso::utilities::geometry::quad_hexahedron QH2 = {{3000,0,0},{6000,0,0,},{3000,3000,0},{6000,3000,0},
														 {3000,0,3000},{6000,0,3000},{3000,3000,3000},{6000,3000,3000}};
		bso::utilities::geometry::quad_hexahedron QH3 = {{-1500,0,0},{1500,0,0,},{-1500,3000,0},{1500,3000,0},
														 {-1500,0,3000},{1500,0,3000},{-1500,3000,3000},{1500,3000,3000}};

		BOOST_REQUIRE(spaceAtGroundV[0].second.isSameAs(QH1));
		BOOST_REQUIRE(spaceAtGroundV[1].second.isSameAs(QH2));
		BOOST_REQUIRE(spaceAtGroundV[2].second.isSameAs(QH3));	
		BOOST_REQUIRE(spaceAtGroundV2[0].second.isSameAs(QH1));
		BOOST_REQUIRE(spaceAtGroundV2[1].second.isSameAs(QH2));
		BOOST_REQUIRE(spaceAtGroundV2[2].second.isSameAs(QH3));										 
	}

	BOOST_AUTO_TEST_CASE( getQuadAtZ0_1 ) 
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,-1500,3000,	1500,-1500,3000,	-1500,1500,3000,	1500,1500,3000,	-1500,-1500,6000,	1500,-1500,6000,	-1500,1500,6000,	1500,1500,6000");

		std::vector<ms_space*> mSpaces;
		mSpaces.push_back(&s1);
		mSpaces.push_back(&s2);
		mSpaces.push_back(&s3);

		std::vector <bso::utilities::geometry::quadrilateral> quads = ms.getQuadAtZ0(mSpaces);

		std::vector<bso::utilities::geometry::quadrilateral> checkQuads = {{{0,0,0},{3000,3000,0},{0,3000,0},{3000,0,0}},{{6000,0,0},{6000,3000,0},{3000,0,0},{3000,3000,0}},
																		 {{-1500,-1500,0},{1500,-1500,0},{1500,1500,0},{-1500,1500,0}}};
		

		BOOST_REQUIRE(quads.size() == 3);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::quadrilateral i: checkQuads)
		{
			bool found = false;
			for(const bso::utilities::geometry::quadrilateral j: quads)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);
	}

	BOOST_AUTO_TEST_CASE( getLinesAtZ0_1 ) 
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,-1500,3000,	1500,-1500,3000,	-1500,1500,3000,	1500,1500,3000,	-1500,-1500,6000,	1500,-1500,6000,	-1500,1500,6000,	1500,1500,6000");

		std::vector<ms_space*> mSpaces;
		mSpaces.push_back(&s1);
		mSpaces.push_back(&s2);
		mSpaces.push_back(&s3);

		std::vector <bso::utilities::geometry::line_segment> lines = ms.getLinesAtZ0(mSpaces);

		std::vector<bso::utilities::geometry::line_segment> checkLines = {{{0,0,0},{3000,0,0}},{{3000,0,0},{3000,3000,0}},{{3000,3000,0},{0,3000,0}},
																		 {{0,3000,0},{0,0,0}},{{3000,0,0},{6000,0,0}},{{6000,0,0},{6000,3000,0}},
																		 {{6000,3000,0},{3000,3000,0}},{{-1500,-1500,0},{1500,-1500,0}},{{1500,-1500,0},{1500,1500,0}},
																		 {{1500,1500,0},{-1500,1500,0}},{{-1500,1500,0},{-1500,-1500,0}}};

		BOOST_REQUIRE(lines.size() == 11);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::line_segment i: checkLines)
		{
			bool found = false;
			for(const bso::utilities::geometry::line_segment j: lines)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);
	}

	BOOST_AUTO_TEST_CASE( getDelaunayVariables_1 )
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,-1500,3000,	1500,-1500,3000,	-1500,1500,3000,	1500,1500,3000,	-1500,-1500,6000,	1500,-1500,6000,	-1500,1500,6000,	1500,1500,6000");

		ms.addSpace(s1);
		ms.addSpace(s2);
		ms.addSpace(s3);

		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV;
		std::vector <bso::utilities::geometry::vertex> delaunayPoints;
		std::vector <bso::utilities::geometry::line_segment> delaunayLines;
		ms.getNOBSDAtGround(spaceAtGroundV);
		ms.getDelaunayVariables(delaunayPoints, delaunayLines, spaceAtGroundV);

		std::vector <bso::utilities::geometry::vertex> checkDelaunayV = {{0,0,0},{3000,0,0},{0,3000,0},{3000,3000,0},{6000,0,0},
																		{6000,3000,0},{1500,1500,0},{-1500,1500,0},{-1500,-1500,0},
																		{1500,-1500,0},{1500,0,0},{0,1500,0}};

		BOOST_REQUIRE(delaunayPoints.size() == 12);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::vertex i: checkDelaunayV)
		{
			bool found = false;
			for(const bso::utilities::geometry::vertex j: delaunayPoints)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);

		std::vector <bso::utilities::geometry::line_segment> checkDelaunayL = {{{0,0,0},{1500,0,0}},{{0,0,0},{0,1500,0}},{{1500,0,0},{1500,1500,0}},{{0,1500,0},{1500,1500,0}},
																		{{1500,0,0},{3000,0,0}},{{0,1500,0},{0,3000,0}},{{3000,0,0},{3000,3000,0}},{{0,3000,0},{3000,3000,0}},
																		{{3000,3000,0},{6000,3000,0}},{{6000,3000,0},{6000,0,0}},{{3000,0,0},{6000,0,0}},{{1500,0,0},{1500,-1500,0}},
																		{{1500,-1500,0},{-1500,-1500,0}},{{-1500,-1500,0,},{-1500,1500,0}},{{-1500,1500,0},{0,1500,0}}};	
			
		BOOST_REQUIRE(delaunayLines.size() == 15);
		anyNotFound = false;
		for(const bso::utilities::geometry::line_segment i: checkDelaunayL)
		{
			bool found = false;
			for(const bso::utilities::geometry::line_segment j: delaunayLines)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);
	}

	/*
	The bellow shiftFillPlane test where performed while developing the shiftFillPlane function.
	If any of the bellow generates an error and a visualization is preferred, the following can be done:
	Go to \BSO-toolbox-V2\unit_tests\spatial_design\NOBSD_test_suite. 
	Here the make file and main.cpp file are there.
	When you compile with 'make cls clean all', followed by running with ./main.
	A visualization will be shown of the bellow boost unit test for shiftFillPlane_x.
	*/

	BOOST_AUTO_TEST_CASE( shiftFillPlane_1 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_327");

		bso::utilities::geometry::vertex location = {7000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_327_Linear_shift");
		BOOST_REQUIRE(msShift == ms);
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_2 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_325");

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,-1,0};
		double shift = sqrt(2) * 3000;
		ms.shiftFillPlane(location, direction, shift);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_325_cornerDiagonalShift");
		BOOST_REQUIRE(msShift == ms);
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_3 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_325");

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = sqrt(2) * 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("spatial_design/NOBSD_test_suite/design_325_cornerFullShift_unit_test");

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_325_cornerFullShift");
		BOOST_REQUIRE(msShift == ms);

		// I have checked the output files and they are exactly the same, so the boolean operation msShift == ms notice a difference???
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_4 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_325");

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_325_cornerMatchShift");
		BOOST_REQUIRE(msShift == ms);
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_5 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_325");

		bso::utilities::geometry::vertex location = {0,6000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_325_shiftSlantedSide");
		BOOST_REQUIRE(msShift == ms);
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_6 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_308");

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		ms.shiftFillPlane(location, direction, shift);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_308_splitLevelAndHorizontalOverlapShift");
		BOOST_REQUIRE(msShift == ms);
	}

	BOOST_AUTO_TEST_CASE( shiftFillPlane_7 )
	{
		bso::spatial_design::ms_N_building ms("spatial_design/NOBSD_test_suite/design_308");

		ms.shiftFillPlaneCentre(5000);

		bso::spatial_design::ms_N_building msShift("spatial_design/NOBSD_test_suite/design_308_centreShiftFillPlane");
		BOOST_REQUIRE(msShift == ms);
	}
	
	
BOOST_AUTO_TEST_SUITE_END()
} // namespace spatial_design_test