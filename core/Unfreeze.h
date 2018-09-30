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

} // namespace sorbet::core
#endif // SORBET_UNFREEZING_H
