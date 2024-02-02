#ifndef BSO_GRAMMAR_SD_TRIPRISM_RULE_HPP
#define BSO_GRAMMAR_SD_TRIPRISM_RULE_HPP

#include <bso/grammar/rule_set/triprism_property.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_rule_set.hpp>


namespace bso { namespace grammar { namespace rule_set { namespace sd_rule_set {

class sd_triprism_rule : public sd_rule_set
{
private:
	triprism_property* mTriprismProperty;

	structural_design::component::structure mStructure;

public:
	sd_triprism_rule(triprism_property* triprismProperty);
	~sd_triprism_rule();

	void apply(bso::structural_design::sd_model& sd) const;
	void assignStructure(structural_design::component::structure structure);
	//void assignLoadPanel(structural_design::component::structure loadPanel);
	//void addLoads(std::multimap<std::string, structural_design::component::load>* loads); // for future, e.g. gravity loads
	const structural_design::component::structure& getStructure() const {return mStructure;}
};

} // namespace sd_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/sd_rule_set/sd_triprism_rule.cpp>

#endif // BSO_GRAMMAR_SD_TRIPRISM_RULE_HPP
