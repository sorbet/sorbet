#ifndef SORBET_CONFIGATRON_H
#define SORBET_CONFIGATRON_H

#include "core/core.h"
namespace sorbet {
namespace namer {

class configatron {
public:
    static void fillInFromFileSystem(core::GlobalState &gs, std::vector<std::string> folders,
                                     std::vector<std::string> files);
};
} // namespace namer
} // namespace sorbet

#endif // SORBET_CONFIGATRON_H
