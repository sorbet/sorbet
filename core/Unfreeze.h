#ifndef SORBET_UNFREEZING_H
#define SORBET_UNFREEZING_H

#include "core.h"

namespace sorbet {
namespace core {

class UnfreezeSymbolTable {
    GlobalState &gs;
    bool oldState;

public:
    UnfreezeSymbolTable(GlobalState &gs);

    ~UnfreezeSymbolTable();
};
class UnfreezeNameTable {
    GlobalState &gs;
    bool oldState;

public:
    UnfreezeNameTable(GlobalState &gs);
    ~UnfreezeNameTable();
};
class UnfreezeFileTable {
    GlobalState &gs;
    bool oldState;

public:
    UnfreezeFileTable(GlobalState &gs);
    ~UnfreezeFileTable();
};

} // namespace core
} // namespace sorbet
#endif // SORBET_UNFREEZING_H
