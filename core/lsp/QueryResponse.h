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
                 bool isPrivateOk, core::SymbolRef enclosingMethod)
        : dispatchResult(std::move(dispatchResult)), callerSideName(callerSideName), termLoc(termLoc),
          isPrivateOk(isPrivateOk), enclosingMethod(enclosingMethod){};
    const std::shared_ptr<core::DispatchResult> dispatchResult;
    const core::NameRef callerSideName;
    const core::Loc termLoc;
    const bool isPrivateOk;
    const core::SymbolRef enclosingMethod;
};

class IdentResponse final {
public:
    IdentResponse(core::Loc termLoc, core::LocalVariable variable, core::TypeAndOrigins retType,
                  core::SymbolRef enclosingMethod)
        : termLoc(termLoc), variable(variable), retType(std::move(retType)), enclosingMethod(enclosingMethod){};
    const core::Loc termLoc;
    const core::LocalVariable variable;
    const core::TypeAndOrigins retType;
    const core::SymbolRef enclosingMethod;
};

class LiteralResponse final {
public:
    LiteralResponse(core::Loc termLoc, core::TypeAndOrigins retType) : termLoc(termLoc), retType(std::move(retType)){};
    const core::Loc termLoc;
    const core::TypeAndOrigins retType;
};

class ConstantResponse final {
public:
    ConstantResponse(core::SymbolRef symbol, core::Loc termLoc, core::NameRef name, core::TypeAndOrigins receiver,
                     core::TypeAndOrigins retType)
        : symbol(symbol), termLoc(termLoc), name(name), retType(std::move(retType)){};
    const core::SymbolRef symbol;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};

class FieldResponse final {
public:
    FieldResponse(core::SymbolRef symbol, core::Loc termLoc, core::NameRef name, core::TypeAndOrigins retType)
        : symbol(symbol), termLoc(termLoc), name(name), retType(std::move(retType)){};
    const core::SymbolRef symbol;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};

class DefinitionResponse final {
public:
    DefinitionResponse(core::SymbolRef symbol, core::Loc termLoc, core::NameRef name, core::TypeAndOrigins retType)
        : symbol(symbol), termLoc(termLoc), name(name), retType(std::move(retType)){};
    const core::SymbolRef symbol;
    const core::Loc termLoc;
    const core::NameRef name;
    const core::TypeAndOrigins retType;
};

class EditResponse final {
public:
    EditResponse(core::Loc loc, std::string replacement) : loc(loc), replacement(std::move(replacement)){};
    const core::Loc loc;
    const std::string replacement;
};

typedef std::variant<SendResponse, IdentResponse, LiteralResponse, ConstantResponse, FieldResponse, DefinitionResponse,
                     EditResponse>
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
