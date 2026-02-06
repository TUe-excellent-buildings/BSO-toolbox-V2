#ifndef GRAMMAR_DL_RECTANGLE_RULE_HPP
#define GRAMMAR_DL_RECTANGLE_RULE_HPP

#include <bso/grammar/rule_set/rectangle_property.hpp>
#include <bso/grammar/rule_set/dl_rule_set/dl_rule_set.hpp>

namespace bso { namespace grammar { namespace rule_set { namespace dl_rule_set {

class dl_rectangle_rule : public dl_rule_set
{
private:
	rectangle_property* mRectangleProperty;
	
public:
	dl_rectangle_rule(rectangle_property* rectangleProperty);
	~dl_rectangle_rule();
	
	void apply(bso::daylight::dl_model& dl) const;
};
	
} // namespace dl_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/dl_rule_set/dl_rectangle_rule.cpp>

#endif // GRAMMAR_DL_RECTANGLE_RULE_HPP