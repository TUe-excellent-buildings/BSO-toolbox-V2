#ifndef BSO_GRAMMAR_SD_TRIANGLE_RULE_HPP
#define BSO_GRAMMAR_SD_TRIANGLE_RULE_HPP

#include <bso/grammar/rule_set/triangle_property.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_rule_set.hpp>

namespace bso { namespace grammar { namespace rule_set { namespace sd_rule_set {

class sd_triangle_rule : public sd_rule_set
{
private:
	triangle_property* mTriangleProperty;
	
	std::multimap<std::string, structural_design::component::load>* mLoads = nullptr;
	structural_design::component::structure mStructure;
	structural_design::component::structure mLoadPanel;
public:
	sd_triangle_rule(triangle_property* triangleProperty);
	~sd_triangle_rule();

	void apply(bso::structural_design::sd_model& sd) const;
	void assignStructure(structural_design::component::structure structure);
	void assignStructure(const std::vector<structural_design::component::structure>&
				potentialStructure);
	void assignLoadPanel(structural_design::component::structure loadPanel);
	void addLoads(std::multimap<std::string, structural_design::component::load>* loads);
	const structural_design::component::structure& getStructure() const {return mStructure;}
};
	
} // namespace sd_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/sd_rule_set/sd_triangle_rule.cpp>

#endif // BSO_GRAMMAR_SD_TRIANGLE_RULE_HPP