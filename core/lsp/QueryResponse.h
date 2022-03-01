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
    SendResponse(core::Loc termLoc, std::shared_ptr<core::DispatchResult> dispatchResult, core::NameRef callerSideName,
                 bool isPrivateOk, core::MethodRef enclosingMethod, core::Loc receiverLoc, core::Loc funLoc,
                 size_t totalArgs)
        : dispatchResult(std::move(dispatchResult)), callerSideName(callerSideName), termLoc(termLoc),
          isPrivateOk(isPrivateOk), enclosingMethod(enclosingMethod), receiverLoc(receiverLoc), funLoc(funLoc),
          totalArgs(totalArgs){};
    const std::shared_ptr<core::DispatchResult> dispatchResult;
    const core::NameRef callerSideName;
    const core::Loc termLoc;
    const bool isPrivateOk;
    const core::MethodRef enclosingMethod;
    const core::Loc receiverLoc;
    const core::Loc funLoc;
    const size_t totalArgs;

    const std::optional<core::Loc> getMethodNameLoc(const core::GlobalState &gs) const;
};
CheckSize(SendResponse, 72, 8);

class IdentResponse final {
public:
    IdentResponse(core::Loc termLoc, core::LocalVariable variable, core::TypeAndOrigins retType,
                  core::MethodRef enclosingMethod)
        : termLoc(termLoc), variable(variable), enclosingMethod(enclosingMethod), retType(std::move(retType)) {}
    const core::Loc termLoc;
    const core::LocalVariable variable;
    const core::MethodRef enclosingMethod;
    const core::TypeAndOrigins retType;
};
CheckSize(IdentResponse, 64, 8);

class LiteralResponse final {
public:
    LiteralResponse(core::Loc termLoc, core::TypeAndOrigins retType) : termLoc(termLoc), retType(std::move(retType)){};
    const core::Loc termLoc;
    const core::TypeAndOrigins retType;
};
CheckSize(LiteralResponse, 56, 8);

class ConstantResponse final {
public:
    using Scopes = InlinedVector<core::SymbolRef, 1>;
    ConstantResponse(core::SymbolRef symbol, core::SymbolRef symbolBeforeDealias, core::Loc termLoc, Scopes scopes,
                     core::NameRef name, core::TypeAndOrigins retType, core::MethodRef enclosingMethod)
        : symbol(symbol), symbolBeforeDealias(symbolBeforeDealias), termLoc(termLoc), scopes(scopes), name(name),
          enclosingMethod(enclosingMethod), retType(std::move(retType)) {}
    const core::SymbolRef symbol;
    // You probably don't want this. Almost all of Sorbet's type system operates on dealiased
    // symbols transparently (e.g., for a constant like `X = Integer`, Sorbet reports that `''` is
    // not an `Integer`, not that `''` is not an `X`).
    //
    // But for some interactive features, it's important to respond using names the user typed.
    // If you're thinking about using this, probably just use `symbol` instead, and if you still
    // think you need this, double check with a Sorbet contributor.
    const core::SymbolRef symbolBeforeDealias;
    const core::Loc termLoc;
    const Scopes scopes;
    const core::NameRef name;
    const core::MethodRef enclosingMethod;
    const core::TypeAndOrigins retType;
};
CheckSize(ConstantResponse, 96, 8);

class FieldResponse final {
public:
    FieldResponse(core::FieldRef symbol, core::Loc termLoc, core::NameRef name, core::TypeAndOrigins retType)
        : symbol(symbol), termLoc(termLoc), name(name), retType(std::move(retType)){};
    const core::FieldRef symbol;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};
CheckSize(FieldResponse, 64, 8);

class DefinitionResponse final {
public:
    DefinitionResponse(core::SymbolRef symbol, core::Loc termLoc, core::NameRef name, core::TypeAndOrigins retType)
        : symbol(symbol), termLoc(termLoc), name(name), retType(std::move(retType)){};
    const core::SymbolRef symbol;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};
CheckSize(DefinitionResponse, 64, 8);

class EditResponse final {
public:
    EditResponse(core::Loc loc, std::string replacement) : loc(loc), replacement(std::move(replacement)){};
    const core::Loc loc;
    const std::string replacement;
};
CheckSize(EditResponse, 40, 8);

using QueryResponseVariant = std::variant<SendResponse, IdentResponse, LiteralResponse, ConstantResponse, FieldResponse,
                                          DefinitionResponse, EditResponse>;

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
     * Returns nullptr unless this is a Field.
     */
    const FieldResponse *isField() const;

    /**
     * Returns nullptr unless this is a Definition.
     */
    const DefinitionResponse *isDefinition() const;

    /**
     * Returns nullptr unless this is an Edit.
     */
    const EditResponse *isEdit() const;

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
     * Returns a reference to this response's TypeAndOrigins, if it has any.
     * If response is of a type without TypeAndOrigins, it throws an exception.
     */
    const core::TypeAndOrigins &getTypeAndOrigins() const;
};

} // namespace sorbet::core::lsp
#endif
