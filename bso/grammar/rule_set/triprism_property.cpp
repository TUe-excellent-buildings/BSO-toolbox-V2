#ifndef BSO_GRAMMAR_TRIPRISM_PROPERTY_CPP
#define BSO_GRAMMAR_TRIPRISM_PROPERTY_CPP

namespace bso { namespace grammar { namespace rule_set {

triprism_property::triprism_property(bso::spatial_design::conformal::cf_triPrism* triprism)
:	mTriprism(triprism)
{
	mIsTriprismProperty = true;
	
	if (mTriprism->cfSpaces().size() > 0)
	{
		mIsInSpace = true;
		mSpaceType = mTriprism->cfSpaces().front()->getSpaceType();
	}
	
	for (const auto& i : *mTriprism)
	{ // iterate trough all vertices
		if (i(2) > 1e-9) // check if anny z is above ground
		{
			mIsAboveGroundSurface = true;
			break;
		}
	}
	mIsBelowGroundSurface = !mIsAboveGroundSurface;

} // ctor()

triprism_property::~triprism_property()
{
	
}
	
} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_TRIPRISM_PROPERTY_CPP