#ifndef SORBET_PARSER_HELPER_H
#define SORBET_PARSER_HELPER_H

#include "parser/parser.h"

namespace sorbet::parser {

/*
 * Helper class for creating parser nodes.
 */
class MK {
public:
    /* nodes */

    /*
     * Create an `[ ... ]` array node.
     */
    static std::unique_ptr<parser::Node> Array(core::LocOffsets loc, parser::NodeVec elements) {
        return std::make_unique<parser::Array>(loc, move(elements));
    }

    /*
     * Create a `::` constant base node.
     */
    static std::unique_ptr<parser::Node> Cbase(core::LocOffsets loc) {
        return std::make_unique<parser::Cbase>(loc);
    }

    /*
     * Create a `Parent::Name` constant node.
     */
    static std::unique_ptr<parser::Node> Const(core::LocOffsets loc, std::unique_ptr<parser::Node> parent,
                                               core::NameRef name) {
        return std::make_unique<parser::Const>(loc, move(parent), name);
    }

    /*
     * Create a `{ ... }` hash node.
     *
     * Use `kwargs = true` if the hash is to be used as a keyword arguments.
     */
    static std::unique_ptr<parser::Node> Hash(core::LocOffsets loc, bool kwargs, parser::NodeVec pairs) {
        return std::make_unique<parser::Hash>(loc, kwargs, move(pairs));
    }

    /*
     * Create a `self` node.
     */
    static std::unique_ptr<parser::Node> Self(core::LocOffsets loc) {
        return std::make_unique<parser::Self>(loc);
    }

    /*
     * Create a `recv.name(args...)` send node.
     */
    static std::unique_ptr<parser::Node> Send(core::LocOffsets loc, std::unique_ptr<parser::Node> recv,
                                              core::NameRef name, core::LocOffsets nameLoc, parser::NodeVec args,
                                              std::unique_ptr<parser::Node> block) {
        return std::make_unique<parser::Send>(loc, move(recv), name, nameLoc, move(args), move(block));
    }

    /*
     * Create a `recv.name()` send node.
     */
    static std::unique_ptr<parser::Node> Send0(core::LocOffsets loc, std::unique_ptr<parser::Node> recv,
                                               core::NameRef name, core::LocOffsets nameLoc) {
        return std::make_unique<parser::Send>(loc, move(recv), name, nameLoc, parser::NodeVec(), nullptr);
    }

    /*
     * Create a `recv.name(arg)` send node.
     */
    static std::unique_ptr<parser::Node> Send1(core::LocOffsets loc, std::unique_ptr<parser::Node> recv,
                                               core::NameRef name, core::LocOffsets nameLoc,
                                               std::unique_ptr<parser::Node> arg) {
        auto args = parser::NodeVec();
        args.reserve(1);
        args.push_back(move(arg));
        return std::make_unique<parser::Send>(loc, move(recv), name, nameLoc, move(args), nullptr);
    }

    /*
     * Create a `recv.name(arg1, arg2)` send node.
     */
    static std::unique_ptr<parser::Node> Send2(core::LocOffsets loc, std::unique_ptr<parser::Node> recv,
                                               core::NameRef name, core::LocOffsets nameLoc,
                                               std::unique_ptr<parser::Node> arg1, std::unique_ptr<parser::Node> arg2) {
        auto args = parser::NodeVec();
        args.reserve(2);
        args.push_back(move(arg1));
        args.push_back(move(arg2));
        return std::make_unique<parser::Send>(loc, move(recv), name, nameLoc, move(args), nullptr);
    }

    /*
     * Create a `"string"` string node.
     */
    static std::unique_ptr<parser::Node> String(core::LocOffsets loc, core::NameRef name) {
        return std::make_unique<parser::String>(loc, name);
    }

    /*
     * Create a `:symbol` symbol node.
     */
    static std::unique_ptr<parser::Node> Symbol(core::LocOffsets loc, core::NameRef name) {
        return std::make_unique<parser::Symbol>(loc, name);
    }

    /*
     * Create a `true` boolean node.
     */
    static std::unique_ptr<parser::Node> True(core::LocOffsets loc) {
        return std::make_unique<parser::True>(loc);
    }

    /* Sorbet classes */

    /*
     * Create a `Sorbet::Private::Static` constant node.
     */
    static std::unique_ptr<parser::Node> SorbetPrivateStatic(core::LocOffsets loc) {
        return std::make_unique<parser::ResolvedConst>(loc, core::Symbols::Sorbet_Private_Static());
    }

    /*
     * Create a `T` constant node.
     */
    static std::unique_ptr<parser::Node> T(core::LocOffsets loc) {
        return Const(loc, Cbase(loc), core::Names::Constants::T());
    }

    /*
     * Create a `T::Array` constant node.
     */
    static std::unique_ptr<parser::Node> T_Array(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Array());
    }

    /*
     * Create a `T::Boolean` constant node.
     */
    static std::unique_ptr<parser::Node> T_Boolean(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Boolean());
    }

    /*
     * Create a `T::Class` constant node.
     */
    static std::unique_ptr<parser::Node> T_Class(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Class());
    }

    /*
     * Create a `T::Enumerable` constant node.
     */
    static std::unique_ptr<parser::Node> T_Enumerable(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Enumerable());
    }

    /*
     * Create a `T::Enumerator` constant node.
     */
    static std::unique_ptr<parser::Node> T_Enumerator(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Enumerator());
    }

    /*
     * Create a `T::Enumerator::Lazy` constant node.
     */
    static std::unique_ptr<parser::Node> T_Enumerator_Lazy(core::LocOffsets loc) {
        return Const(loc, T_Enumerator(loc), core::Names::Constants::Lazy());
    }

    /*
     * Create a `T::Enumerator::Chain` constant node.
     */
    static std::unique_ptr<parser::Node> T_Enumerator_Chain(core::LocOffsets loc) {
        return Const(loc, T_Enumerator(loc), core::Names::Constants::Chain());
    }

    /*
     * Create a `T::Hash` constant node.
     */
    static std::unique_ptr<parser::Node> T_Hash(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Hash());
    }

    /*
     * Create a `T::Helpers` constant node.
     */
    static std::unique_ptr<parser::Node> T_Helpers(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Helpers());
    }

    /*
     * Create a `T::Set` constant node.
     */
    static std::unique_ptr<parser::Node> T_Set(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Set());
    }

    /*
     * Create a `T::Sig::WithoutRuntime` resolved constant node.
     */
    static std::unique_ptr<parser::Node> T_Sig_WithoutRuntime(core::LocOffsets loc) {
        return std::make_unique<parser::ResolvedConst>(loc, core::Symbols::T_Sig_WithoutRuntime());
    }

    /*
     * Create a `T::Range` constant node.
     */
    static std::unique_ptr<parser::Node> T_Range(core::LocOffsets loc) {
        return Const(loc, T(loc), core::Names::Constants::Range());
    }

    /* T methods */

    /*
     * Create a `T.absurd()` send node.
     */
    static std::unique_ptr<parser::Node> TAbsurd(core::LocOffsets loc, std::unique_ptr<parser::Node> node) {
        return Send1(loc, T(loc), core::Names::absurd(), loc, move(node));
    }

    /*
     * Create a `T.all(args...)` send node.
     */
    static std::unique_ptr<parser::Node> TAll(core::LocOffsets loc, parser::NodeVec args) {
        return Send(loc, T(loc), core::Names::all(), loc, move(args), nullptr);
    }

    /*
     * Create a `T.any(args...)` send node.
     */
    static std::unique_ptr<parser::Node> TAny(core::LocOffsets loc, parser::NodeVec args) {
        return Send(loc, T(loc), core::Names::any(), loc, move(args), nullptr);
    }

    /*
     * Create a `T.anything()` send node.
     */
    static std::unique_ptr<parser::Node> TAnything(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::anything(), loc);
    }

    /*
     * Create a `T.attached_class()` send node.
     */
    static std::unique_ptr<parser::Node> TAttachedClass(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::attachedClass(), loc);
    }

    static std::unique_ptr<parser::Node> TBindSelf(core::LocOffsets loc, std::unique_ptr<parser::Node> type) {
        return Send2(loc, T(loc), core::Names::bind(), loc, MK::Self(loc), move(type));
    }

    /*
     * Create a `T.cast(value, type)` send node.
     */
    static std::unique_ptr<parser::Node> TCast(core::LocOffsets loc, std::unique_ptr<parser::Node> value,
                                               std::unique_ptr<parser::Node> type) {
        return Send2(loc, T(loc), core::Names::cast(), loc, move(value), move(type));
    }

    /*
     * Create a `T.class_of(type)` send node.
     */
    static std::unique_ptr<parser::Node> TClassOf(core::LocOffsets loc, std::unique_ptr<parser::Node> type) {
        return Send1(loc, T(loc), core::Names::classOf(), loc, move(type));
    }

    /*
     * Create a `T.let(value, type)` send node.
     */
    static std::unique_ptr<parser::Node> TLet(core::LocOffsets loc, std::unique_ptr<parser::Node> value,
                                              std::unique_ptr<parser::Node> type) {
        return Send2(loc, T(loc), core::Names::let(), loc, move(value), move(type));
    }

    /*
     * Create a `T.must(value)` send node.
     */
    static std::unique_ptr<parser::Node> TMust(core::LocOffsets loc, std::unique_ptr<parser::Node> value) {
        return Send1(loc, T(loc), core::Names::must(), loc, move(value));
    }

    /*
     * Create a `T.nilable(type)` send node.
     */
    static std::unique_ptr<parser::Node> TNilable(core::LocOffsets loc, std::unique_ptr<parser::Node> type) {
        return Send1(loc, T(loc), core::Names::nilable(), loc, move(type));
    }

    /*
     * Create a `T.noreturn()` send node.
     */
    static std::unique_ptr<parser::Node> TNoReturn(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::noreturn(), loc);
    }

    /*
     * Create a `T.proc.params(args...).returns(returnType)` send node.
     */
    static std::unique_ptr<parser::Node> TProc(core::LocOffsets loc, std::unique_ptr<parser::Hash> args,
                                               std::unique_ptr<parser::Node> returnType) {
        auto builder = T(loc);
        builder = Send0(loc, move(builder), core::Names::proc(), loc);
        if (args != nullptr && !args->pairs.empty()) {
            auto argsVec = parser::NodeVec();
            argsVec.reserve(1);
            argsVec.push_back(move(args));
            builder = Send(loc, move(builder), core::Names::params(), loc, move(argsVec), nullptr);
        }
        builder = Send1(loc, move(builder), core::Names::returns(), loc, move(returnType));
        return builder;
    }

    /*
     * Create a `T.proc.params(args...).void` send node.
     */
    static std::unique_ptr<parser::Node> TProcVoid(core::LocOffsets loc, std::unique_ptr<parser::Hash> args) {
        auto builder = T(loc);
        builder = Send0(loc, move(builder), core::Names::proc(), loc);
        if (args != nullptr && !args->pairs.empty()) {
            auto argsVec = parser::NodeVec();
            argsVec.reserve(1);
            argsVec.push_back(move(args));
            builder = Send(loc, move(builder), core::Names::params(), loc, move(argsVec), nullptr);
        }
        return Send0(loc, move(builder), core::Names::void_(), loc);
    }

    /*
     * Create a `T.self_type()` send node.
     */
    static std::unique_ptr<parser::Node> TSelfType(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::selfType(), loc);
    }

    /*
     * Create a `T.type_alias { type }` send node.
     */
    static std::unique_ptr<parser::Node> TTypeAlias(core::LocOffsets loc, std::unique_ptr<parser::Node> type) {
        auto send = Send0(loc, T(loc), core::Names::typeAlias(), loc);
        auto body = std::make_unique<parser::Begin>(loc, parser::NodeVec());
        body->stmts.push_back(move(type));
        parser::cast_node<parser::Send>(send.get())->block = std::make_unique<parser::Block>(loc, nullptr, move(body));
        return send;
    }

    /*
     * Create a `T.type_parameter(:name)` send node.
     */
    static std::unique_ptr<parser::Node> TTypeParameter(core::LocOffsets loc, std::unique_ptr<parser::Node> name) {
        return Send1(loc, T(loc), core::Names::typeParameter(), loc, move(name));
    }

    /*
     * Create a `T.unsafe(value)` send node.
     */
    static std::unique_ptr<parser::Node> TUnsafe(core::LocOffsets loc, std::unique_ptr<parser::Node> value) {
        return Send1(loc, T(loc), core::Names::unsafe(), loc, move(value));
    }

    /*
     * Create a `T.untyped()` send node.
     */
    static std::unique_ptr<parser::Node> TUntyped(core::LocOffsets loc) {
        return std::make_unique<parser::Send>(loc, T(loc), core::Names::untyped(), loc, parser::NodeVec(), nullptr);
    }

    /* is helpers */

    /*
     * Is `expr` a `T` or `::T` constant node?
     */
    static bool isT(const std::unique_ptr<parser::Node> &expr) {
        if (expr == nullptr) {
            return false;
        }

        auto t = parser::cast_node<parser::Const>(expr.get());
        return t != nullptr && t->name == core::Names::Constants::T() &&
               (t->scope == nullptr || isa_node<parser::Cbase>(t->scope.get()));
    }

    /*
     * Is `expr` a `T.untyped()` or `::T.untyped()` send node?
     */
    static bool isTUntyped(const std::unique_ptr<parser::Node> &expr) {
        auto send = parser::cast_node<parser::Send>(expr.get());
        return send != nullptr && send->method == core::Names::untyped() && isT(send->receiver);
    }

    /**
     * Is `send` a visibility modifier (private, protected, public, etc.)?
     */
    static bool isVisibilitySend(const parser::Send *send) {
        return send->receiver == nullptr && send->args.size() == 1 &&
               (parser::isa_node<parser::DefMethod>(send->args[0].get()) ||
                parser::isa_node<parser::DefS>(send->args[0].get())) &&
               (send->method == core::Names::private_() || send->method == core::Names::protected_() ||
                send->method == core::Names::public_() || send->method == core::Names::privateClassMethod() ||
                send->method == core::Names::publicClassMethod() || send->method == core::Names::packagePrivate() ||
                send->method == core::Names::packagePrivateClassMethod());
    }

    /**
     * Is `send` an attribute accessor (attr_reader, attr_writer, attr_accessor)?
     */
    static bool isAttrAccessorSend(const parser::Send *send) {
        return (send->receiver == nullptr || parser::isa_node<parser::Self>(send->receiver.get())) &&
               (send->method == core::Names::attrReader() || send->method == core::Names::attrWriter() ||
                send->method == core::Names::attrAccessor());
    }
};

} // namespace sorbet::parser

#endif // SORBET_PARSER_HELPER_H
