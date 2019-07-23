// have to be included first as they violate our poisons
#include "cfg/proto/proto.h"

#include "absl/container/inlined_vector.h"
#include "common/typecase.h"
#include "core/proto/proto.h"

using namespace std;

namespace sorbet::cfg {

com::stripe::rubytyper::LocalVariable Proto::toProto(const core::GlobalState &gs, const core::LocalVariable &var) {
    com::stripe::rubytyper::LocalVariable proto;
    proto.set_unique_name(var.exportableName(gs));
    return proto;
}

com::stripe::rubytyper::LocalVariable Proto::toProto(const core::GlobalState &gs, const VariableUseSite &vus) {
    auto proto = toProto(gs, vus.variable);
    if (vus.type) {
        *proto.mutable_type() = core::Proto::toProto(gs, vus.type);
    }
    return proto;
}

InlinedVector<com::stripe::rubytyper::Binding, 1> Proto::toProto(const core::GlobalState &gs, const Binding &bnd) {
    vector<com::stripe::rubytyper::Binding> protos;
    typecase(
        bnd.value.get(),
        [&](const Ident *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::IDENT);
            *instr.mutable_ident() = toProto(gs, i->what);

            *protos.back().mutable_instruction() = instr;
        },
        [&](const Alias *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::ALIAS);
            instr.set_alias_full_name(i->what.show(gs));

            *protos.back().mutable_instruction() = instr;
        },
        [&](const Send *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::SEND);
            *instr.mutable_send()->mutable_receiver() = toProto(gs, i->recv);
            instr.mutable_send()->set_method(i->fun.show(gs));
            if (i->link) {
                *instr.mutable_send()->mutable_block() = com::stripe::rubytyper::Instruction::Block();
            }
            for (const auto &arg : i->args) {
                *instr.mutable_send()->add_arguments() = toProto(gs, arg);
            }

            *protos.back().mutable_instruction() = instr;
        },
        [&](const TAbsurd *i) {
            com::stripe::rubytyper::Binding aliasBinding;
            *aliasBinding.mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::LocalVariable local;
            local.set_unique_name(core::Names::tConstTemp().show(gs));
            *local.mutable_type() = core::Proto::toProto(
                gs, core::make_type<core::ClassType>(core::Symbols::T().data(gs)->lookupSingletonClass(gs)));
            *aliasBinding.mutable_bind() = local;

            com::stripe::rubytyper::Instruction alias;
            alias.set_kind(com::stripe::rubytyper::Instruction::ALIAS);
            alias.set_alias_full_name(core::Symbols::T().show(gs));
            *aliasBinding.mutable_instruction() = alias;

            protos.emplace_back(aliasBinding);

            com::stripe::rubytyper::Binding sendBinding;
            *sendBinding.mutable_location() = core::Proto::toProto(gs, bnd.loc);

            *sendBinding.mutable_bind() = toProto(gs, bnd.bind);

            com::stripe::rubytyper::Instruction send;
            send.set_kind(com::stripe::rubytyper::Instruction::SEND);
            *send.mutable_send()->mutable_receiver() = local;
            send.mutable_send()->set_method(core::Names::absurd().show(gs));
            *send.mutable_send()->add_arguments() = toProto(gs, i->what);
            *sendBinding.mutable_instruction() = send;

            protos.emplace_back(sendBinding);
        },
        [&](const Return *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::RETURN);
            *instr.mutable_return_() = toProto(gs, i->what);

            *protos.back().mutable_instruction() = instr;
        },
        [&](const Literal *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::LITERAL);
            *instr.mutable_literal() = core::Proto::toProto(gs, i->value);

            *protos.back().mutable_instruction() = instr;
        },
        [&](const Unanalyzable *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::UNANALYZABLE);

            *protos.back().mutable_instruction() = instr;
        },
        [&](const LoadArg *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::LOAD_ARG);
            instr.set_load_arg_name(i->argument(gs).argumentName(gs));

            *protos.back().mutable_instruction() = instr;
        },
        [&](const Cast *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::CAST);
            *instr.mutable_cast()->mutable_value() = toProto(gs, i->value);
            *instr.mutable_cast()->mutable_type() = core::Proto::toProto(gs, i->type);

            *protos.back().mutable_instruction() = instr;
        },
        // TODO later: add more types
        [&](const Instruction *i) {
            protos.emplace_back();
            *protos.back().mutable_bind() = toProto(gs, bnd.bind);
            *protos.back().mutable_location() = core::Proto::toProto(gs, bnd.loc);

            com::stripe::rubytyper::Instruction instr;
            instr.set_kind(com::stripe::rubytyper::Instruction::UNKNOWN);

            *protos.back().mutable_instruction() = instr;
        });

    return protos;
}

com::stripe::rubytyper::Block::BlockExit Proto::toProto(const core::GlobalState &gs, const BlockExit &ex) {
    com::stripe::rubytyper::Block::BlockExit proto;
    if (ex.cond.variable.exists()) {
        *proto.mutable_cond() = toProto(gs, ex.cond);
    }
    if (ex.thenb) {
        proto.set_then_block(ex.thenb->id);
    }
    if (ex.elseb) {
        proto.set_else_block(ex.elseb->id);
    }
    *proto.mutable_location() = core::Proto::toProto(gs, ex.loc);
    return proto;
}

com::stripe::rubytyper::Block Proto::toProto(const core::GlobalState &gs, const BasicBlock &bb) {
    com::stripe::rubytyper::Block proto;
    proto.set_id(bb.id);
    for (auto const &bnd : bb.exprs) {
        for (auto const &bndProto : toProto(gs, bnd)) {
            *proto.add_bindings() = bndProto;
        }
    }
    *proto.mutable_block_exit() = toProto(gs, bb.bexit);
    return proto;
}

com::stripe::rubytyper::CFG::Argument Proto::argumentToProto(const core::GlobalState &gs, const core::ArgInfo &sym) {
    com::stripe::rubytyper::CFG::Argument proto;

    proto.set_name(sym.argumentName(gs));
    if (sym.type) {
        *proto.mutable_type() = core::Proto::toProto(gs, sym.type);
    }
    return proto;
}

com::stripe::rubytyper::CFG Proto::toProto(const core::GlobalState &gs, const CFG &cfg) {
    com::stripe::rubytyper::CFG proto;
    core::SymbolData sym = cfg.symbol.data(gs);

    proto.set_definition_full_name(cfg.symbol.show(gs));
    *proto.mutable_location() = core::Proto::toProto(gs, sym->loc());

    if (sym->resultType) {
        *proto.mutable_return_type() = core::Proto::toProto(gs, sym->resultType);
    }

    for (auto &arg : sym->arguments()) {
        *proto.add_arguments() = argumentToProto(gs, arg);
    }

    for (auto const &block : cfg.basicBlocks) {
        *proto.add_blocks() = toProto(gs, *block);
    }
    return proto;
}

com::stripe::rubytyper::MultiCFG Proto::toMulti(const com::stripe::rubytyper::CFG &cfg) {
    com::stripe::rubytyper::MultiCFG proto;
    *proto.add_cfg() = cfg;
    return proto;
}

} // namespace sorbet::cfg
