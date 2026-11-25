#ifndef SORBET_UNFREEZING_H
#define SORBET_UNFREEZING_H

#include "core.h"

namespace sorbet::core {

class UnfreezeSymbolTable {
    GlobalState &gs;

public:
    UnfreezeSymbolTable(GlobalState &gs);

    ~UnfreezeSymbolTable();
};
class UnfreezeNameTable {
    GlobalState &gs;

public:
    UnfreezeNameTable(GlobalState &gs);
    ~UnfreezeNameTable();
};
class UnfreezeFileTable {
    GlobalState &gs;

public:
    UnfreezeFileTable(GlobalState &gs);
    ~UnfreezeFileTable();
};

// Forbids entering new symbols, while still allowing existing symbols to be found and updated. This is only checked
// in debug builds.
class ForbidNewSymbols {
    GlobalState &gs;
    bool old;

public:
    ForbidNewSymbols(GlobalState &gs);
    ~ForbidNewSymbols();
};

} // namespace sorbet::core
#endif // SORBET_UNFREEZING_H
