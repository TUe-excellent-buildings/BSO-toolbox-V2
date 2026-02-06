#ifndef GRAMMAR_DL_RULE_SET_HPP
#define GRAMMAR_DL_RULE_SET_HPP

namespace bso { namespace grammar { namespace rule_set { namespace dl_rule_set {

class dl_rule_set
{
private:
	
public:
	dl_rule_set();
	~dl_rule_set();
	
	virtual void apply(bso::daylight::dl_model& dl) const = 0;

};
	
} // namespace dl_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/dl_rule_set/dl_rule_set.cpp>

#endif // GRAMMAR_DL_RULE_SET_HPP