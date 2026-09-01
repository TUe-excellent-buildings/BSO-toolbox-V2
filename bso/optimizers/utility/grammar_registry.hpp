#ifndef BSO_OPTIMIZERS_UTILITY_GRAMMAR_REGISTRY_HPP
#define BSO_OPTIMIZERS_UTILITY_GRAMMAR_REGISTRY_HPP

#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/structural_design/sd_model.hpp>

// Central registries mapping a grammar-choice string (as given via the -gram
// command line argument) to the code that applies the corresponding grammar.
// Shared across all optimizers under bso/optimizers.
//
// To add a new grammar and make it selectable via -gram <type> <name>:
//   1. Implement the grammar itself (e.g. a new window/sd/bp/dl grammar class).
//   2. Add one entry to the relevant registry below.
// No optimizer code needs to change.
namespace bso { namespace optimizers { namespace utility {

using WindowGeomGrammarFn = std::function<spatial_design::cf_building(
    const spatial_design::ms_N_building&, const std::string&, double)>;
using WindowGrammarFn = std::function<void(grammar::grammar&)>;
using SDGrammarFn = std::function<structural_design::sd_model(grammar::grammar&, const std::string&)>;
using BPGrammarFn = std::function<building_physics::bp_model(grammar::grammar&, const std::string&)>;
using DLGrammarFn = std::function<daylight::dl_model(grammar::grammar&, const std::string&, const std::string&)>;

inline const std::map<std::string, WindowGeomGrammarFn>& windowGeomGrammarRegistry()
{
    static const std::map<std::string, WindowGeomGrammarFn> registry = {
        {"default_window_geometry", [](const spatial_design::ms_N_building& ms, const std::string& dlSettingsFile, double tolerance)
            {
                return spatial_design::cf_building(ms, dlSettingsFile,
                    spatial_design::conformal::window_geometry_grammar_tag<spatial_design::conformal::DEFAULT_WINDOW_GEOMETRY_GRAMMAR>(),
                    tolerance);
            }}
    };
    return registry;
}

inline const std::map<std::string, WindowGrammarFn>& windowGrammarRegistry()
{
    static const std::map<std::string, WindowGrammarFn> registry = {
        {"default_window", [](grammar::grammar& grm) { grm.applyWindowGrammar<grammar::DEFAULT_WINDOW_GRAMMAR>(); }}
    };
    return registry;
}

inline const std::map<std::string, SDGrammarFn>& sdGrammarRegistry()
{
    static const std::map<std::string, SDGrammarFn> registry = {
        {"default_sd", [](grammar::grammar& grm, const std::string& settingsFile)
            { return grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(settingsFile); }}
    };
    return registry;
}

inline const std::map<std::string, BPGrammarFn>& bpGrammarRegistry()
{
    static const std::map<std::string, BPGrammarFn> registry = {
        {"default_bp", [](grammar::grammar& grm, const std::string& settingsFile)
            { return grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(settingsFile); }}
    };
    return registry;
}

inline const std::map<std::string, DLGrammarFn>& dlGrammarRegistry()
{
    static const std::map<std::string, DLGrammarFn> registry = {
        {"default_dl", [](grammar::grammar& grm, const std::string& settingsFile, const std::string& dlFolder)
            { return grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(settingsFile, dlFolder); }}
    };
    return registry;
}

// Looks up 'choice' in 'registry', throwing a clear error naming 'grammarKind' if not found.
template <typename GrammarFn>
const GrammarFn& lookupGrammar(const std::map<std::string, GrammarFn>& registry,
    const std::string& choice, const std::string& grammarKind)
{
    auto it = registry.find(choice);
    if (it == registry.end())
    {
        std::stringstream errorMessage;
        errorMessage << "Error, unknown " << grammarKind << " grammar choice: \"" << choice << "\".\n"
                     << "(bso/optimizers/utility/grammar_registry.hpp)" << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    return it->second;
}

}}} // namespace bso::optimizers::utility

#endif // BSO_OPTIMIZERS_UTILITY_GRAMMAR_REGISTRY_HPP
