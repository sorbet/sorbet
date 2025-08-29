#include "Unfreeze.h"

namespace sorbet::core {

UnfreezeSymbolTable::UnfreezeSymbolTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeSymbolTable();
    ENFORCE(oldState, "The symbol table was already unfrozen. Remove the excess call to UnfreezeSymbolTable.");
}

UnfreezeSymbolTable::~UnfreezeSymbolTable() {
    gs.freezeSymbolTable();
}

UnfreezeNameTable::UnfreezeNameTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeNameTable();
    ENFORCE(oldState, "The name table was already unfrozen. Remove the excess call to UnfreezeNameTable.");
}

UnfreezeNameTable::~UnfreezeNameTable() {
    gs.freezeNameTable();
}

UnfreezeFileTable::UnfreezeFileTable(GlobalState &gs) : gs(gs) {
    auto oldState = gs.unfreezeFileTable();
    ENFORCE(oldState, "The file table was already unfrozen. Remove the excess call to UnfreezeFileTable.");
}

UnfreezeFileTable::~UnfreezeFileTable() {
    gs.freezeFileTable();
}

} // namespace sorbet::core
