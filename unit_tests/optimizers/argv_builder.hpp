#ifndef ARGV_BUILDER_HPP
#define ARGV_BUILDER_HPP

#include <initializer_list>
#include <string>
#include <vector>

namespace {
struct ArgvBuilder {
    std::vector<std::string> args;
    std::vector<char*> cargs;

    ArgvBuilder(std::initializer_list<const char*> init)
    {
        args.reserve(init.size());
        cargs.reserve(init.size());
        for (const auto* item : init) {
            args.emplace_back(item);
        }
        for (auto& item : args) {
            cargs.push_back(&item[0]);
        }
    }

    int argc() const { return static_cast<int>(args.size()); }
    char** argv() { return cargs.data(); }
};
} // namespace

#endif // ARGV_BUILDER_HPP
