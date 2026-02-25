#ifndef BSO_GRAMMAR_SD_RULE_SETS_HPP
#define BSO_GRAMMAR_SD_RULE_SETS_HPP

#include <bso/grammar/rule_set/sd_rule_set/sd_vertex_rule.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_line_rule.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_rectangle_rule.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_triangle_rule.hpp>
#include <bso/grammar/rule_set/sd_rule_set/sd_cuboid_rule.hpp>
//#include <bso/grammar/rule_set/sd_rule_set/sd_triprism_rule.hpp>

namespace bso { namespace grammar { namespace rule_set {

typedef sd_rule_set::sd_vertex_rule sd_vertex_rule;
typedef sd_rule_set::sd_line_rule sd_line_rule;
typedef sd_rule_set::sd_rectangle_rule sd_rectangle_rule;
typedef sd_rule_set::sd_triangle_rule sd_triangle_rule;
typedef sd_rule_set::sd_cuboid_rule sd_cuboid_rule;
// The sd_triprism_rule class is not needed at the moment. When solid elements 
// are needed to represent the NOBSD representation (i.e. ms_N_building), this representation can be used
// See cuboid_rule_set for example how to implement this.
// typedef sd_rule_set::sd_cuboid_rule sd_triprism_rule; 

} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_SD_RULE_SETS_HPP
