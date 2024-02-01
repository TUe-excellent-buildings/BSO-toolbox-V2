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
		
		std::vector<bso::utilities::geometry::vertex*> pVertex  = s1->getVertices();
		
		bso::utilities::geometry::vertex p1 = {0,0,0};
		bso::utilities::geometry::vertex p2 = {6000,0,0};
		bso::utilities::geometry::vertex p3 = {0,6000,0};
		bso::utilities::geometry::vertex p4 = {6000,6000,0};
		bso::utilities::geometry::vertex p5 = {0,0,6000};
		bso::utilities::geometry::vertex p6 = {6000,0,6000};
		bso::utilities::geometry::vertex p7 = {0,6000,6000};
		bso::utilities::geometry::vertex p8 = {6000,6000,6000};	
		
		bool verticesCheck = true;
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p1) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p2) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p3) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p4) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p5) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p6) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p7) == *(pVertex.end())){verticesCheck = false;}
		if(std::find(*(pVertex.begin()),*(pVertex.end()),p8) == *(pVertex.end())){verticesCheck = false;}																			   

	BOOST_REQUIRE(verticesCheck = true);
		BOOST_REQUIRE(sstr.str() == "N,1,	0,0,0,	6000,0,0,	0,6000,0,	6000,6000,0,	0,0,6000,	6000,0,6000,	0,6000,6000,	6000,6000,6000");
		BOOST_REQUIRE(b1.getSpacePtrs().size() == 1);
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
		
		std::vector <bso::utilities::geometry::vertex*> pVert;
		std::vector <bso::utilities::geometry::vertex*> pVert1;
		std::vector <bso::utilities::geometry::vertex*> pVert2;
		std::vector <bso::utilities::geometry::vertex*> pVert3;

		// pVert space 0
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {1000,0,0};
		bso::utilities::geometry::vertex v3 = {1000,2000,0};
		bso::utilities::geometry::vertex v4 = {0,2000,0};
		bso::utilities::geometry::vertex v5 = {1000,0,3000};
		bso::utilities::geometry::vertex v6 = {1000,2000,3000};
		bso::utilities::geometry::vertex v7 = {1000,2000,3000};
		bso::utilities::geometry::vertex v8 = {0,2000,3000};
		pVert.push_back(&v1);
		pVert.push_back(&v2);
		pVert.push_back(&v3);
		pVert.push_back(&v4);
		pVert.push_back(&v5);
		pVert.push_back(&v6);
		pVert.push_back(&v7);
		pVert.push_back(&v8);

		// pVert space 1
		bso::utilities::geometry::vertex v11 = {0,0,0};
		bso::utilities::geometry::vertex v21 = {1000,0,0};
		bso::utilities::geometry::vertex v31 = {1000,2000,0};
		bso::utilities::geometry::vertex v41 = {0,2000,0};
		bso::utilities::geometry::vertex v51 = {1000,0,3000};
		bso::utilities::geometry::vertex v61 = {1000,2000,3000};
		bso::utilities::geometry::vertex v71 = {1000,2000,3000};
		bso::utilities::geometry::vertex v81 = {0,2000,3000};
		pVert1.push_back(&v11);
		pVert1.push_back(&v21);
		pVert1.push_back(&v31);
		pVert1.push_back(&v41);
		pVert1.push_back(&v51);
		pVert1.push_back(&v61);
		pVert1.push_back(&v71);
		pVert1.push_back(&v81);

		// pVert space 2
		bso::utilities::geometry::vertex v12 = {4000,0,0};
		bso::utilities::geometry::vertex v22 = {5000,1000,0};
		bso::utilities::geometry::vertex v32 = {5000,3000,0};
		bso::utilities::geometry::vertex v42 = {4000,3000,0};
		bso::utilities::geometry::vertex v52 = {4000,0,3000};
		bso::utilities::geometry::vertex v62 = {5000,1000,3000};
		bso::utilities::geometry::vertex v72 = {5000,3000,3000};
		bso::utilities::geometry::vertex v82 = {4000,3000,3000};
		pVert2.push_back(&v12);
		pVert2.push_back(&v22);
		pVert2.push_back(&v32);
		pVert2.push_back(&v42);
		pVert2.push_back(&v52);
		pVert2.push_back(&v62);
		pVert2.push_back(&v72);
		pVert2.push_back(&v82);

		// pVert space 3
		bso::utilities::geometry::vertex v13 = {4000,0,0};
		bso::utilities::geometry::vertex v23 = {5000,1000,0};
		bso::utilities::geometry::vertex v33 = {5000,3000,0};
		bso::utilities::geometry::vertex v43 = {4000,3000,0};
		bso::utilities::geometry::vertex v53 = {4000,0,3000};
		bso::utilities::geometry::vertex v63 = {5000,1000,3000};
		bso::utilities::geometry::vertex v73 = {5000,3000,3000};
		bso::utilities::geometry::vertex v83 = {4000,3000,3000};
		pVert3.push_back(&v13);
		pVert3.push_back(&v23);
		pVert3.push_back(&v33);
		pVert3.push_back(&v43);
		pVert3.push_back(&v53);
		pVert3.push_back(&v63);
		pVert3.push_back(&v73);
		pVert3.push_back(&v83);

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
		
		std::vector <bso::utilities::geometry::vertex*> pVert;
		
		bso::utilities::geometry::vertex v1 = {0,0,1000};
		bso::utilities::geometry::vertex v2 = {2000,0,1000};
		bso::utilities::geometry::vertex v3 = {0,3000,1000};
		bso::utilities::geometry::vertex v4 = {2000,2000,1000};
		bso::utilities::geometry::vertex v5 = {2000,0,2000};
		bso::utilities::geometry::vertex v6 = {0,3000,2000};
		bso::utilities::geometry::vertex v7 = {2000,2000,2000};
		bso::utilities::geometry::vertex v8 = {2000,0,2000};
		pVert.push_back(&v1);
		pVert.push_back(&v2);
		pVert.push_back(&v3);
		pVert.push_back(&v4);
		pVert.push_back(&v5);
		pVert.push_back(&v6);
		pVert.push_back(&v7);
		pVert.push_back(&v8);

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
		
		Eigen::Vector3d coord1(0,0,0), coord2(0,0,4303);
		Eigen::Vector3d dim1(3000,3000,3000);

		// space 1
		std::vector <bso::utilities::geometry::vertex*> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {3000,0,0};
		bso::utilities::geometry::vertex v3 = {3000,3000,0};
		bso::utilities::geometry::vertex v4 = {0,3000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {3000,0,3000};
		bso::utilities::geometry::vertex v7 = {3000,3000,3000};
		bso::utilities::geometry::vertex v8 = {0,3000,3000};
		pVert.push_back(&v1);
		pVert.push_back(&v2);
		pVert.push_back(&v3);
		pVert.push_back(&v4);
		pVert.push_back(&v5);
		pVert.push_back(&v6);
		pVert.push_back(&v7);
		pVert.push_back(&v8);

		// space 2
		std::vector <bso::utilities::geometry::vertex*> pVert2;
		bso::utilities::geometry::vertex v12 = {0,0,4303};
		bso::utilities::geometry::vertex v22 = {3000,0,4303};
		bso::utilities::geometry::vertex v32 = {0,3000,4303};
		bso::utilities::geometry::vertex v42 = {3000,3000,4303};
		bso::utilities::geometry::vertex v52 = {3000,0,7303};
		bso::utilities::geometry::vertex v62 = {0,3000,7303};
		bso::utilities::geometry::vertex v72 = {3000,3000,7303};
		bso::utilities::geometry::vertex v82 = {0,0,7303};
		pVert2.push_back(&v12);
		pVert2.push_back(&v22);
		pVert2.push_back(&v32);
		pVert2.push_back(&v42);
		pVert2.push_back(&v52);
		pVert2.push_back(&v62);
		pVert2.push_back(&v72);
		pVert2.push_back(&v82);
		
		b1.addSpace(ms_space(1,pVert));
		b1.addSpace(ms_space(2,pVert2));

		BOOST_REQUIRE(b1 != b2);
		BOOST_REQUIRE(b2 != b1);

		b2.setZZero();
		
		BOOST_REQUIRE(b1 == b2);
		BOOST_REQUIRE(b2 == b1);
	}
	
	BOOST_AUTO_TEST_CASE( addSpace_and_Comparison_2 )
	{
		ms_N_building b1;
		ms_N_building b2;

		// space 1
		std::vector <bso::utilities::geometry::vertex*> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {3000,0,0};
		bso::utilities::geometry::vertex v3 = {3000,3000,0};
		bso::utilities::geometry::vertex v4 = {0,3000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {3000,0,3000};
		bso::utilities::geometry::vertex v7 = {3000,3000,3000};
		bso::utilities::geometry::vertex v8 = {0,3000,3000};
		pVert.push_back(&v1);
		pVert.push_back(&v2);
		pVert.push_back(&v3);
		pVert.push_back(&v4);
		pVert.push_back(&v5);
		pVert.push_back(&v6);
		pVert.push_back(&v7);
		pVert.push_back(&v8);

		// space 2; the same as space 1 however with diferent definition order of vertices
		std::vector <bso::utilities::geometry::vertex*> pVert2;
		bso::utilities::geometry::vertex v12 = {3000,3000,3000};
		bso::utilities::geometry::vertex v22 = {3000,3000,0};
		bso::utilities::geometry::vertex v32 = {3000,0,0};
		bso::utilities::geometry::vertex v42 = {0,3000,0};
		bso::utilities::geometry::vertex v52 = {0,0,3000};
		bso::utilities::geometry::vertex v62 = {3000,0,3000};
		bso::utilities::geometry::vertex v72 = {0,0,0};
		bso::utilities::geometry::vertex v82 = {0,3000,3000};
		pVert2.push_back(&v12);
		pVert2.push_back(&v22);
		pVert2.push_back(&v32);
		pVert2.push_back(&v42);
		pVert2.push_back(&v52);
		pVert2.push_back(&v62);
		pVert2.push_back(&v72);
		pVert2.push_back(&v82);
		
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
	
	BOOST_AUTO_TEST_CASE( deleteSpace )
	{
		ms_N_building b1("spatial_design/ms_N_test_1.txt");
		ms_N_building b2;
		
		// space 1
		std::vector <bso::utilities::geometry::vertex*> pVert;
		bso::utilities::geometry::vertex v1 = {0,0,0};
		bso::utilities::geometry::vertex v2 = {1000,0,0};
		bso::utilities::geometry::vertex v3 = {1000,2000,0};
		bso::utilities::geometry::vertex v4 = {0,2000,0};
		bso::utilities::geometry::vertex v5 = {0,0,3000};
		bso::utilities::geometry::vertex v6 = {1000,0,3000};
		bso::utilities::geometry::vertex v7 = {1000,2000,3000};
		bso::utilities::geometry::vertex v8 = {0,2000,3000};
		pVert.push_back(&v1);
		pVert.push_back(&v2);
		pVert.push_back(&v3);
		pVert.push_back(&v4);
		pVert.push_back(&v5);
		pVert.push_back(&v6);
		pVert.push_back(&v7);
		pVert.push_back(&v8);

		// space 2
		std::vector <bso::utilities::geometry::vertex*> pVert2;
		bso::utilities::geometry::vertex v12 = {1000,0,0};
		bso::utilities::geometry::vertex v22 = {4000,0,0};
		bso::utilities::geometry::vertex v32 = {1000,4000,0};
		bso::utilities::geometry::vertex v42 = {4000,4000,0};
		bso::utilities::geometry::vertex v52 = {1000,0,3000};
		bso::utilities::geometry::vertex v62 = {4000,0,3000};
		bso::utilities::geometry::vertex v72 = {1000,4000,3000};
		bso::utilities::geometry::vertex v82 = {4000,4000,3000};
		pVert2.push_back(&v12);
		pVert2.push_back(&v22);
		pVert2.push_back(&v32);
		pVert2.push_back(&v42);
		pVert2.push_back(&v52);
		pVert2.push_back(&v62);
		pVert2.push_back(&v72);
		pVert2.push_back(&v82);

		// space 3
		std::vector <bso::utilities::geometry::vertex*> pVert3;
		bso::utilities::geometry::vertex v13 = {0,0,4303};
		bso::utilities::geometry::vertex v23 = {3000,0,4303};
		bso::utilities::geometry::vertex v33 = {0,3000,4303};
		bso::utilities::geometry::vertex v43 = {3000,3000,4303};
		bso::utilities::geometry::vertex v53 = {3000,0,7303};
		bso::utilities::geometry::vertex v63 = {0,3000,7303};
		bso::utilities::geometry::vertex v73 = {3000,3000,7303};
		bso::utilities::geometry::vertex v83 = {0,0,7303};
		pVert3.push_back(&v13);
		pVert3.push_back(&v23);
		pVert3.push_back(&v33);
		pVert3.push_back(&v43);
		pVert3.push_back(&v53);
		pVert3.push_back(&v63);
		pVert3.push_back(&v73);
		pVert3.push_back(&v83);


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
		std::stringstream sstr, check1, check2; 
		
		b1.scale({{0,1.5},{1,2},{2,1.23}});
		sstr << b1;
		check1 << "N,1,	0,0,857.31,	4500,0,857.31,	0,6000,857.31,	4500,6000,857.31,	0,0,4547.31,	4500,0,4547.31,	0,6000,4547.31,	4500,6000,4547.31" << std::endl
					<< "N,2,	0,0,6150,	4500,0,6150,	0,6000,6150,	4500,6000,6150,	0,0,9840,	4500,0,9840,	0,6000,9840,	4500,6000,9840" ;
		BOOST_REQUIRE(sstr.str() == check1.str());
		sstr.str(std::string());

		b1.scale();
		sstr << b1;
		check2 << "N,1,	0,0,857.31,	6363.96,0,857.31,	0,8485.28,857.31,	6363.96,8485.28,857.31,	0,0,4547.31,	6363.96,0,4547.31,	0,8485.28,4547.31,	6363.96,8485.28,4547.31" << std::endl
					<< "N,2,	0,0,6150,	6363.96,0,6150,	0,8485.28,6150,	6363.96,8485.28,6150,	0,0,9840,	6363.96,0,9840,	0,8485.28,9840,	6363.96,8485.28,9840";
		BOOST_REQUIRE(sstr.str() == check2.str());
		
		BOOST_REQUIRE_THROW(b1.scale({{3,1.5}}), std::invalid_argument);
		BOOST_REQUIRE_THROW(b1.scale({{1,1.0},{1,1.0}}), std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE( scale_2 )
	{ // test of an non-orthogonal design for scaling
		ms_N_building b1("spatial_design/ms_N_test_9.txt");
		std::stringstream sstr, check1, check2; 
		
		b1.scale({{0,1.5},{1,2},{2,1.98}});
		sstr << b1;
		check1 << "N,1,	0,0,0,	4500,0,0,	0,8000,0,	6000,8000,0,	0,0,5940,	4500,0,5940,	0,8000,5940,	6000,8000,5940" << std::endl
					<< "N,2,	0,0,5940,	4500,-4000,5940,	0,6000,5940,	4500,6000,5940,	0,0,15840,	4500,-4000,15840,	0,6000,15840,	4500,6000,15840" ;
		BOOST_REQUIRE(sstr.str() == check1.str());
		sstr.str(std::string());

		b1.scale();
		sstr << b1;
		check2 << "N,1,	0,0,0,	6363.96,0,0,	0,11313.7,0,	8485.28,11313.7,0,	0,0,5940,	6363.96,0,5940,	0,11313.7,5940,	8485.28,11313.7,5940" << std::endl
					<< "N,2,	0,0,5940,	6363.96,-5656.85,5940,	0,8485.28,5940,	6363.96,8485.28,5940,	0,0,15840,	6363.96,-5656.85,15840,	0,8485.28,15840,	6363.96,8485.28,15840" ;
		BOOST_REQUIRE(sstr.str() == check2.str());
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
	
	
BOOST_AUTO_TEST_SUITE_END()
} // namespace spatial_design_test