#ifndef SORBET_LSP_QUERYRESPONSE
#define SORBET_LSP_QUERYRESPONSE
#include "core/Loc.h"
#include "core/LocalVariable.h"
#include "core/Types.h"
#include <variant>

namespace sorbet::core::lsp {
class TypeConstraint;

class SendResponse final {
public:
    SendResponse(core::DispatchResult::ComponentVec dispatchComponents,
                 const std::shared_ptr<core::TypeConstraint> &constraint, core::Loc termLoc, core::NameRef name,
                 core::TypeAndOrigins receiver, core::TypeAndOrigins retType);
    core::DispatchResult::ComponentVec dispatchComponents;
    const std::shared_ptr<core::TypeConstraint> constraint;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins receiver;
    const core::TypeAndOrigins retType;
};

class IdentResponse final {
public:
    IdentResponse(core::SymbolRef owner, core::Loc termLoc, core::LocalVariable variable, core::TypeAndOrigins retType);
    const core::SymbolRef owner;
    const core::Loc termLoc;
    const core::LocalVariable variable;
    const core::TypeAndOrigins retType;
};

class LiteralResponse final {
public:
    LiteralResponse(core::SymbolRef owner, core::Loc termLoc, core::TypeAndOrigins retType);
    const core::SymbolRef owner;
    const core::Loc termLoc;
    const core::TypeAndOrigins retType;
};

class ConstantResponse final {
public:
    ConstantResponse(core::SymbolRef owner, core::DispatchResult::ComponentVec dispatchComponents, core::Loc termLoc,
                     core::NameRef name, core::TypeAndOrigins receiver, core::TypeAndOrigins retType);
    const core::SymbolRef owner;
    core::DispatchResult::ComponentVec dispatchComponents;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins receiver;
    const core::TypeAndOrigins retType;
};

class DefinitionResponse final {
public:
    DefinitionResponse(core::DispatchResult::ComponentVec dispatchComponents, core::Loc termLoc, core::NameRef name,
                       core::TypeAndOrigins retType);
    core::DispatchResult::ComponentVec dispatchComponents;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};

typedef std::variant<SendResponse, IdentResponse, LiteralResponse, ConstantResponse, DefinitionResponse>
    QueryResponseVariant;

/**
 * Represents a response to a LSP query. Wraps a variant that contains one of several response types.
 */
class QueryResponse final {
private:
    QueryResponseVariant response;

public:
    /**
     * Pushes the given query response on to the error queue.
     */
    static void pushQueryResponse(core::Context ctx, QueryResponseVariant rawResponse);

    QueryResponse(QueryResponseVariant response);

    /**
     * Returns nullptr unless this is a Send.
     */
    const SendResponse *isSend() const;

    /**
     * Returns nullptr unless this is an Ident.
     */
    const IdentResponse *isIdent() const;

    /**
     * Returns nullptr unless this is a Literal.
     */
    const LiteralResponse *isLiteral() const;

    /**
     * Returns nullptr unless this is a Constant.
     */
    const ConstantResponse *isConstant() const;

    /**
     * Returns nullptr unless this is a Definition.
     */
    const DefinitionResponse *isDefinition() const;

    /**
     * Returns the source code location for the specific expression that this
     * response points to.
     */
    core::Loc getLoc() const;

    /**
     * Returns the type of this expression's rval.
     */
    core::TypePtr getRetType() const;

    /**
     * Returns dispatch components associated with this response, if any.
     * If none, returns an empty vector.
     */
    const core::DispatchResult::ComponentVec &getDispatchComponents() const;
};

} // namespace sorbet::core::lsp
#endif