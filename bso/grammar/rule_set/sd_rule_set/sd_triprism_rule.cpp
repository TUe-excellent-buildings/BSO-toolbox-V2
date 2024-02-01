#ifndef BSO_GRAMMAR_SD_TRIPRISM_RULE_CPP
#define BSO_GRAMMAR_SD_TRIPRISM_RULE_CPP

namespace bso { namespace grammar { namespace rule_set { namespace sd_rule_set {

sd_triprism_rule::sd_triprism_rule(triprism_property* triprismProperty)
:   mTriprismProperty(triprismProperty), mStructure(structural_design::component::structure())
{

}//ctor()

sd_triprism_rule::~sd_triprism_rule()
{

}//dtor()


void sd_triprism_rule::apply(bso::structural_design::sd_model& sd) const
{ //NOTE: this void is not updated yet and is based on the rectangle version
    /*
    structural_design::component::geometry* quadGeometry = nullptr;
	if (mStructure.type() == "quad_hexahedron")
    { //create a guad_hexahedron at the location of the cuboid
        quadGeometry = sd.addGeometry(*(mCuboidProperty->getCuboidPtr()));
        quadGeometry->addStructure(mStructure);
    }
    */
}

void sd_triprism_rule::assignStructure(structural_design::component::structure structure)
	{
	    mStructure = structure;
	}

} // namespace sd_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_SD_TRIPRISM_RULE_CPP
