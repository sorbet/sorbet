#include "Unfreeze.h"

namespace sorbet {
namespace core {

UnfreezeSymbolTable::UnfreezeSymbolTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeSymbolTable();
    ENFORCE(oldState);
}

UnfreezeSymbolTable::~UnfreezeSymbolTable() {
    gs.freezeSymbolTable();
}

UnfreezeNameTable::UnfreezeNameTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeNameTable();
    ENFORCE(oldState);
}

UnfreezeNameTable::~UnfreezeNameTable() {
    gs.freezeNameTable();
}

UnfreezeFileTable::UnfreezeFileTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeFileTable();
    ENFORCE(oldState);
}

UnfreezeFileTable::~UnfreezeFileTable() {
    gs.freezeFileTable();
}

} // namespace core
} // namespace sorbet
