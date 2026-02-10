#ifndef BSO_GRAMMAR_OPTIMIZER_ASSIGNMENT_CPP
#define BSO_GRAMMAR_OPTIMIZER_ASSIGNMENT_CPP

#include <bso/utilities/clustering.hpp>
#include <stdexcept>
#include <sstream>

namespace bso { namespace grammar {
class OPTIMIZER_ASSIGNMENT;

template <>
bso::structural_design::sd_model grammar::sd_grammar<OPTIMIZER_ASSIGNMENT>(
	const std::string& fileName,
	const std::string& gaDesign,
	const bso::structural_design::component::structure& trussStructure,
	const bso::structural_design::component::structure& beamStructure,
	const bso::structural_design::component::structure& flatShellStructure)
{
	// initialize a new SD model
	mSDModel = bso::structural_design::sd_model();
	
	// read the structural design settings
	this->mReadSDSettings(fileName);

	// initialize rectangle rules
	for (const auto& i : mRectangleProperties)
	{
		if (i.second->isSpaceSeparating())
		{
			// create a rectangle rule set
			auto sdRectangleRuleSet = new rule_set::sd_rectangle_rule(i.second);
			mSDRectangleRules[i.first] = sdRectangleRuleSet;
			sdRectangleRuleSet->assignLoadPanel(mLoadPanel);
			sdRectangleRuleSet->addLoads(&mLoads);
		}
	}
	// initialize triangle rules
	for (const auto& i : mTriangleProperties)
	{
		if (i.second->isSpaceSeparating())
		{
			// create a triangle rule set
			auto sdTriangleRuleSet = new rule_set::sd_triangle_rule(i.second);
			mSDTriangleRules[i.first] = sdTriangleRuleSet;
			sdTriangleRuleSet->assignLoadPanel(mLoadPanel);
			sdTriangleRuleSet->addLoads(&mLoads);
		}
	}
	
	// check if the number of passed arguments is correct
	if (gaDesign.size() != (mSDRectangleRules.size() + mSDTriangleRules.size()))
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, when running the design via optimizer assignment.\n"
								 << "Number of input variables: << " << gaDesign.size()
								 << "\ndoes not match number of eligible rectangles in conformal model: "
								 << (mSDRectangleRules.size() + mSDTriangleRules.size()) << "\n"
								 << "(bso/grammar/sd_grammar/optimizer_assignment.cpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	// assign each rectangle rule the assigned type
	auto variableIterator = gaDesign.begin();
	for (auto& i : mSDRectangleRules)
	{
		switch(*variableIterator)
		{
			case '1':
			{ // no structure
				i.second->assignStructure(bso::structural_design::component::structure());
				break;
			}
			case '2':
			{ // trusses
				i.second->assignStructure(trussStructure);
				break;
			}
			case '3':
			{ // beams
				i.second->assignStructure(beamStructure);
				break;
			}
			case '4':
			{ // flat shells
				i.second->assignStructure(flatShellStructure);
				break;
			}
			default:
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, when running the design via optimizer assignment.\n"
										 << "Cannot assign structural type to rectangle,\n"
										 << "did not recognize argument: " << *variableIterator
										 << "\n(bso/grammar/sd_grammar/optimizer_assignment.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
		++variableIterator;
	}
	// assign each triangle rule the assigned type
	for (auto& i : mSDTriangleRules)
	{
		switch(*variableIterator)
		{
			case '1':
			{ // no structure
				i.second->assignStructure(bso::structural_design::component::structure());
				break;
			}
			case '2':
			{ // trusses
				i.second->assignStructure(trussStructure);
				break;
			}
			case '3':
			{ // beams
				i.second->assignStructure(beamStructure);
				break;
			}
			case '4':
			{ // flat shells
				i.second->assignStructure(flatShellStructure);
				break;
			}
			default:
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, when running the design via optimizer assignment.\n"
										 << "Cannot assign structural type to rectangle,\n"
										 << "did not recognize argument: " << *variableIterator
										 << "\n(bso/grammar/sd_grammar/optimizer_assignment.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
		++variableIterator;
	}
	
	// find each line that is associated to a rectangle that has a rectangle rule set
	for (const auto& i : mSDRectangleRules)
	{ // for each sd rectangle rule
		for (const auto& j : i.first->cfLines())
		{ // for each line associated to the rectangle associated to the rectangle rule
			if (mSDLineRules.find(j) == mSDLineRules.end());
			{ // if it has not yet been added to the vector, add it
				mSDLineRules[j] = new rule_set::sd_line_rule(mLineProperties[j]);
			}
		}
	}
	// find each line that is associated to a triangle that has a triangle rule set
	for (const auto& i : mSDTriangleRules)
	{ // for each sd rectangle rule
		for (const auto& j : i.first->cfLines())
		{ // for each line associated to the rectangle associated to the rectangle rule
			if (mSDLineRules.find(j) == mSDLineRules.end());
			{ // if it has not yet been added to the vector, add it
				mSDLineRules[j] = new rule_set::sd_line_rule(mLineProperties[j]);
			}
		}
	}
	
	// find the structural type for each line segment
	for (auto& i : mSDLineRules)
	{ // for each sd line rule
		// find the potential structural types from all the possible sd rectangle rules this
		// line rule may be associated with
		std::vector<structural_design::component::structure> potentialStructures;
		for (const auto& j : i.first->cfRectangles())
		{ // for each rectangle that this line is associated with
			auto SDRectangleRuleSearch = mSDRectangleRules.find(j);
			if (SDRectangleRuleSearch != mSDRectangleRules.end())
			{ // if that rectangle is associated with an sd rectangle rule, store its structural type
				potentialStructures.push_back(SDRectangleRuleSearch->second->getStructure());
			}
		}
		for (const auto& j : i.first->cfTriangles())
		{ // for each triangle that this line is associated with
			auto SDTriangleRuleSearch = mSDTriangleRules.find(j);
			if (SDTriangleRuleSearch != mSDTriangleRules.end())
			{ // if that triangle is associated with an sd triangle rule, store its structural type
				potentialStructures.push_back(SDTriangleRuleSearch->second->getStructure());
			}
		}

		// pass the potential structural types to the sd line rule
		i.second->assignStructure(potentialStructures);
	}

	// initialize vertex rules
	for (const auto& i : mVertexProperties)
	{
		if (i.second->isSpaceCorner())
		{
			mSDVertexRules[i.first] = new rule_set::sd_vertex_rule(i.second);
		}
	}
	
	// apply the rules
	for (const auto& j : mSDVertexRules) j.second->apply(mSDModel);
	for (const auto& j : mSDLineRules) j.second->apply(mSDModel);
	for (const auto& j : mSDRectangleRules) j.second->apply(mSDModel);
	for (const auto& j : mSDTriangleRules) j.second->apply(mSDModel);
	mSDModel.setMeshSize(mMeshSize);
	
	// delete the rules
	for (auto& i : mSDVertexRules) delete i.second;
	for (auto& i : mSDLineRules) delete i.second;
	for (auto& i : mSDRectangleRules) delete i.second;
	for (auto& i : mSDTriangleRules) delete i.second;

	// clear the maps to prevent dangling pointers
	mSDVertexRules.clear();
	mSDLineRules.clear();
	mSDRectangleRules.clear();
	mSDTriangleRules.clear();
	
	return mSDModel;
} // default_sd_grammar

} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_OPTIMIZER_ASSIGNMENT_CPP