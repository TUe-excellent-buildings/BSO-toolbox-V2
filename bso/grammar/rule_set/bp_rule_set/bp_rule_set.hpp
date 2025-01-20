#ifndef BSO_GRAMMAR_BP_RULE_SET_HPP
#define BSO_GRAMMAR_BP_RULE_SET_HPP

namespace bso { namespace grammar { namespace rule_set { namespace bp_rule_set {

class bp_rule_set
{
private:
	
public:
	bp_rule_set();
	virtual ~bp_rule_set(); // virtual destructor to make sure it is called when a derived class destructor is called
	
	virtual void apply(bso::building_physics::bp_model& bp) const = 0;

};
	
} // namespace bp_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/bp_rule_set/bp_rule_set.cpp>

#endif // BSO_GRAMMAR_BP_RULE_SET_HPP