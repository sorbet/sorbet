#ifndef SRUBY_UNFREEZING_H
#define SRUBY_UNFREEZING_H

#include "core.h"

namespace ruby_typer {
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
} // namespace ruby_typer
#endif // SRUBY_UNFREEZING_H
