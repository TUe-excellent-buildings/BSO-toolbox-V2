#ifndef BSO_GRAMMAR_TRIPRISM_PROPERTY_HPP
#define BSO_GRAMMAR_TRIPRISM_PROPERTY_HPP

#include <bso/grammar/rule_set/property_base.hpp>

namespace bso { namespace grammar { namespace rule_set {

class triprism_property : public property_base
{
private:
	bso::spatial_design::conformal::cf_triPrism* mTriprism;
	
	std::string mSpaceType = "";
	bool mIsInSpace = false;
	bool mIsAboveGroundSurface = false;
	bool mIsBelowGroundSurface = false;
public:
	triprism_property(bso::spatial_design::conformal::cf_triPrism* triprism);
	~triprism_property();
	
	bso::spatial_design::conformal::cf_triPrism* getTriprismPtr() const {return mTriprism;}
	const std::string& getSpaceType() const {return mSpaceType;}
	
	const bool& isInSpace() const {return mIsInSpace;}
	const bool& isAboveGroundSurface() const {return mIsAboveGroundSurface;}
	const bool& isBelowGroundSurface() const {return mIsBelowGroundSurface;}
};
	
} // namespace rule_set
} // namespace grammar
} // namespace bso

#include <bso/grammar/rule_set/triprism_property.cpp>

#endif // BSO_GRAMMAR_TRIPRISM_PROPERTY_HPP