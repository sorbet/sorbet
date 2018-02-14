#ifndef SRUBY_CONFIGATRON_H
#define SRUBY_CONFIGATRON_H

#include "core/core.h"
namespace ruby_typer {
namespace namer {

class configatron {
public:
    static void fillInFromFileSystem(core::GlobalState &gs, std::vector<std::string> folders,
                                     std::vector<std::string> files);
};
} // namespace namer
} // namespace ruby_typer

#endif // SRUBY_CONFIGATRON_H
