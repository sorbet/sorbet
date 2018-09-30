#ifndef SORBET_CONFIGATRON_H
#define SORBET_CONFIGATRON_H

#include "core/core.h"
namespace sorbet::namer {

class configatron {
public:
    static void fillInFromFileSystem(core::GlobalState &gs, std::vector<std::string> folders,
                                     std::vector<std::string> files);
};
} // namespace sorbet::namer

#endif // SORBET_CONFIGATRON_H
