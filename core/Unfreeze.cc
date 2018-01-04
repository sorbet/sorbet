#include "Unfreeze.h"

ruby_typer::core::UnfreezeSymbolTable::UnfreezeSymbolTable(ruby_typer::core::GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeSymbolTable();
}

ruby_typer::core::UnfreezeSymbolTable::~UnfreezeSymbolTable() {
    if (oldState) {
        gs.freezeSymbolTable();
    }
}

ruby_typer::core::UnfreezeNameTable::UnfreezeNameTable(ruby_typer::core::GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeNameTable();
}

ruby_typer::core::UnfreezeNameTable::~UnfreezeNameTable() {
    if (oldState) {
        gs.freezeNameTable();
    }
}

ruby_typer::core::UnfreezeFileTable::UnfreezeFileTable(ruby_typer::core::GlobalState &gs) : gs(gs) {
    this->oldState = gs.unfreezeFileTable();
}

ruby_typer::core::UnfreezeFileTable::~UnfreezeFileTable() {
    if (oldState) {
        gs.freezeFileTable();
    }
}
