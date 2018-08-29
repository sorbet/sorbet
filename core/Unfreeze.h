#ifndef SORBET_UNFREEZING_H
#define SORBET_UNFREEZING_H

#include "core.h"

namespace sorbet {
namespace core {

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

} // namespace core
} // namespace sorbet
#endif // SORBET_UNFREEZING_H
