#ifndef SORBET_CFG_PROTO_H
#define SORBET_CFG_PROTO_H
// have to go first as they violate our poisons
#include "proto/CFG.pb.h"

#include "cfg/CFG.h"

namespace sorbet::cfg {
class Proto {
public:
    Proto() = delete;

    static com::stripe::rubytyper::LocalVariable toProto(const core::GlobalState &gs, const VariableUseSite &vus);
    static com::stripe::rubytyper::LocalVariable toProto(const core::GlobalState &gs, const core::LocalVariable &var);

    static com::stripe::rubytyper::Instruction toProto(const core::GlobalState &gs, const Instruction *inst);

    static com::stripe::rubytyper::Binding toProto(const core::GlobalState &gs, const Binding &bnd);

    static com::stripe::rubytyper::Block::BlockExit toProto(const core::GlobalState &gs, const BlockExit &ex);
    static com::stripe::rubytyper::Block toProto(const core::GlobalState &gs, const BasicBlock &bb);

    static com::stripe::rubytyper::CFG::Argument argumentToProto(const core::GlobalState &gs, const core::ArgInfo &sym);
    static com::stripe::rubytyper::CFG toProto(const core::GlobalState &gs, const CFG &cfg);
    static com::stripe::rubytyper::MultiCFG toMulti(const com::stripe::rubytyper::CFG &cfg);
};

} // namespace sorbet::cfg

#endif
