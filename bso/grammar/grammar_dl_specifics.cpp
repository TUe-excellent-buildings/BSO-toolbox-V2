#ifndef GRAMMAR_DL_SPECIFICS_CPP
#define GRAMMAR_DL_SPECIFICS_CPP

#include <bso/utilities/trim_and_cast.hpp>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <stdexcept>
#include <sstream>

namespace bso { namespace grammar {

void grammar::mReadDLSettings(const std::string& fileName)
{
	std::fstream input(fileName.c_str());
	if (!input.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "\nCould not read DL grammar settings, failed to open file:"
								 << fileName << "\n(bso/grammar/grammar_dl_specifics.cpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	//here it clears DLproperties
	//and properties are defined
	mReflectanceValues.clear();
	mWindowProperties.clear();
	mWindowDimensions.clear();
	mDF_Settings.clear();
	mDF_Evaluation.clear();

	std::string line;
	boost::char_separator<char> sep(","); // defines what separates tokens in a string
	typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
	char type_ID; // holds information about what type of information is described by the line currently read

	while(!input.eof())	//Get properties from file
	{
		getline(input, line); // get the next line from the file
		boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
		if (line == "") //skip empty lines (tokenizer does not like it)
		{
			continue; // continue to next line
		}
		t_tokenizer tok(line, sep); // tokenize the line
		t_tokenizer::iterator token = tok.begin(); // set iterator to first token

		try
		{
			type_ID = bso::utilities::trim_and_cast_char(*token);	++token; // interpret first token as type ID
		}
		catch (std::exception& e)
		{
			continue;
		}

		switch (type_ID)
		{
			case 'A':	//reflectance values
			{
				std::string ReflectanceValueFloor = *token; ++token;
				boost::algorithm::trim(ReflectanceValueFloor);
				std::string ReflectanceValueCeiling = *token; ++token;
				boost::algorithm::trim(ReflectanceValueCeiling);
				std::string ReflectanceValueWall = *token; ++token;
				boost::algorithm::trim(ReflectanceValueWall);
				std::string Specularity = *token; ++token;
				boost::algorithm::trim(Specularity);
				std::string Roughness = *token; ++token;
				boost::algorithm::trim(Roughness);		
				mReflectanceValues = {ReflectanceValueFloor,ReflectanceValueCeiling,ReflectanceValueWall, Specularity, Roughness};
				break;
			}
			case 'B':
			{ // window properties
				std::string transmissivity = *token; ++token;
				boost::algorithm::trim(transmissivity);
				std::string transmittance = *token; ++token;
				boost::algorithm::trim(transmittance);
				mWindowProperties = {transmissivity,transmittance};
				break;
			}
			case 'C':
			{ // window dimensions
				double WindowHeightRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				double WindowWidthRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				double StartLocationWindow = bso::utilities::trim_and_cast_double(*token); ++token;
				mWindowDimensions = {WindowHeightRatio,WindowWidthRatio,StartLocationWindow};
				break;
			}
			case 'D':
			{ // DF settings
				double HorizontalOutdoorIlluminance = bso::utilities::trim_and_cast_double(*token); ++token;
				double ReferenceHeight = bso::utilities::trim_and_cast_double(*token); ++token;
				double space_margin = bso::utilities::trim_and_cast_double(*token); ++token;
				double minGridSize = bso::utilities::trim_and_cast_double(*token); ++token;
				mDF_Settings = {HorizontalOutdoorIlluminance,ReferenceHeight,space_margin,minGridSize};
				break;
			}			
			case 'E':
			{ // DF evaluation
				double Dt = bso::utilities::trim_and_cast_double(*token); ++token;
				double Dtm = bso::utilities::trim_and_cast_double(*token); ++token;
				mDF_Evaluation = {Dt,Dtm};
				break;
			}	
		}
	

	} //get properties from file
 

	input.close();
} // mReadDLSettings()

} // namespace grammar
} // namespace bso

#endif // GRAMMAR_DL_SPECIFICS_CPP
