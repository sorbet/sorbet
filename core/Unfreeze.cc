#include "Unfreeze.h"

namespace sorbet {
namespace core {

UnfreezeSymbolTable::UnfreezeSymbolTable(GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeSymbolTable();
}

UnfreezeSymbolTable::~UnfreezeSymbolTable() {
    if (oldState) {
        gs.freezeSymbolTable();
    }
}

UnfreezeNameTable::UnfreezeNameTable(GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeNameTable();
}

UnfreezeNameTable::~UnfreezeNameTable() {
    if (oldState) {
        gs.freezeNameTable();
    }
}

UnfreezeFileTable::UnfreezeFileTable(GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeFileTable();
}

UnfreezeFileTable::~UnfreezeFileTable() {
    if (oldState) {
        gs.freezeFileTable();
    }
}

} // namespace core
} // namespace sorbet
