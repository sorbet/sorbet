#include "parser/prism/Factory.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"

namespace sorbet::parser::Prism {

using namespace std;
using namespace std::literals::string_view_literals;

// See Prism's `include/prism/defines.h`
inline constexpr auto prismFree = [](void *p) { xfree(p); };

// A unique_ptr to memory that will be owned by Prism.
// * Must be allocated from one of the Prism allocator functions
//   See Prism's `include/prism/defines.h`
// * Ownership can be transfered to a Prism-owned AST via `release()`.
// * In the event of an exception, will be freed correctly using Prism's `xfree()`.
template <typename T> using PrismUniquePtr = std::unique_ptr<T, decltype(prismFree)>;

pm_node_list_t Factory::copyNodesToList(const vector<pm_node_t *> &nodes) const {
    auto size = nodes.size();

    if (size == 0) {
        return (pm_node_list_t){.size = 0, .capacity = 0, .nodes = nullptr};
    }

    auto result = static_cast<pm_node_t **>(this->calloc(size, sizeof(pm_node_t *)));
    for (size_t i = 0; i < size; i++) {
        result[i] = nodes[i];
    }
    return (pm_node_list_t){.size = size, .capacity = size, .nodes = result};
}

pm_arguments_node_t *Factory::createArgumentsNode(vector<pm_node_t *> args, const pm_location_t loc) const {
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();

    pm_node_list_t argNodes = copyNodesToList(args);

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE, loc), .arguments = argNodes};

    return arguments;
}

pm_node_t Factory::initializeBaseNode(pm_node_type_t type, const pm_location_t loc) const {
    pm_parser_t *prismParser = parser.getRawParserPointer();
    prismParser->node_id++;
    uint32_t nodeId = prismParser->node_id;

    return (pm_node_t){.type = type, .flags = 0, .node_id = nodeId, .location = loc};
}

pm_node_t *Factory::ConstantReadNode(string_view name, core::LocOffsets loc) const {
    pm_constant_id_t constantId = addConstantToPool(name);
    return ConstantReadNode(constantId, loc);
}

pm_node_t *Factory::ConstantReadNode(pm_constant_id_t constantId, core::LocOffsets loc) const {
    pm_constant_read_node_t *node = allocateNode<pm_constant_read_node_t>();

    pm_location_t pmLoc = parser.convertLocOffsets(loc);
    *node = (pm_constant_read_node_t){.base = initializeBaseNode(PM_CONSTANT_READ_NODE, pmLoc), .name = constantId};

    return up_cast(node);
}

pm_node_t *Factory::ConstantWriteNode(core::LocOffsets loc, pm_constant_id_t nameId, pm_node_t *value) const {
    ENFORCE(value, "ConstantWriteNode: value is required");

    pm_constant_write_node_t *node = allocateNode<pm_constant_write_node_t>();

    pm_location_t pmLoc = parser.convertLocOffsets(loc);
    pm_location_t zeroLoc = parser.getZeroWidthLocation();

    *node = (pm_constant_write_node_t){.base = initializeBaseNode(PM_CONSTANT_WRITE_NODE, pmLoc),
                                       .name = nameId,
                                       .name_loc = pmLoc,
                                       .value = value,
                                       .operator_loc = zeroLoc};

    return up_cast(node);
}

pm_node_t *Factory::ConstantPathNode(core::LocOffsets loc, pm_node_t *parent, string_view name) const {
    pm_constant_id_t nameId = addConstantToPool(name);
    pm_constant_path_node_t *node = allocateNode<pm_constant_path_node_t>();

    pm_location_t pmLoc = parser.convertLocOffsets(loc);

    *node = (pm_constant_path_node_t){.base = initializeBaseNode(PM_CONSTANT_PATH_NODE, pmLoc),
                                      .parent = parent,
                                      .name = nameId,
                                      .delimiter_loc = pmLoc,
                                      .name_loc = pmLoc};

    return up_cast(node);
}

pm_node_t *Factory::SingleArgumentNode(pm_node_t *arg) const {
    ENFORCE(arg, "SingleArgumentNode: arg is required");

    vector<pm_node_t *> args = {arg};
    pm_arguments_node_t *arguments = createArgumentsNode(args, arg->location);

    return up_cast(arguments);
}

pm_node_t *Factory::Self(core::LocOffsets loc) const {
    ENFORCE(loc.exists(), "Self: location is required");

    pm_self_node_t *selfNode = allocateNode<pm_self_node_t>();

    *selfNode = (pm_self_node_t){.base = initializeBaseNode(PM_SELF_NODE, parser.convertLocOffsets(loc))};

    return up_cast(selfNode);
}

pm_node_t *Factory::True(core::LocOffsets loc) const {
    ENFORCE(loc.exists(), "True: location is required");

    pm_true_node_t *trueNode = allocateNode<pm_true_node_t>();

    *trueNode = (pm_true_node_t){.base = initializeBaseNode(PM_TRUE_NODE, parser.convertLocOffsets(loc))};

    return up_cast(trueNode);
}

pm_constant_id_t Factory::addConstantToPool(string_view name) const {
    pm_parser_t *prismParser = parser.getRawParserPointer();
    size_t nameLen = name.size();
    PrismUniquePtr<uint8_t> stable{reinterpret_cast<uint8_t *>(this->calloc(nameLen, sizeof(uint8_t))), prismFree};

    memcpy(stable.get(), name.data(), nameLen);
    pm_constant_id_t id = pm_constant_pool_insert_owned(&prismParser->constant_pool, stable.release(), nameLen);
    return id;
}

pm_call_node_t *Factory::createSendNode(pm_node_t *receiver, pm_constant_id_t methodId, pm_node_t *arguments,
                                        pm_location_t messageLoc, pm_location_t fullLoc, pm_location_t tinyLoc,
                                        pm_node_t *block) const {
    pm_call_node_t *call = allocateNode<pm_call_node_t>();

    *call = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE, fullLoc),
                             .receiver = receiver,
                             .call_operator_loc = tinyLoc,
                             .name = methodId,
                             .message_loc = messageLoc,
                             .opening_loc = tinyLoc,
                             .arguments = down_cast<pm_arguments_node_t>(arguments),
                             .closing_loc = tinyLoc,
                             .block = block};

    return call;
}

pm_node_t *Factory::SymbolFromConstant(core::LocOffsets nameLoc, pm_constant_id_t nameId) const {
    auto nameView = parser.resolveConstant(nameId);
    size_t nameSize = nameView.size();

    PrismUniquePtr<uint8_t> stable{reinterpret_cast<uint8_t *>(this->calloc(nameSize, sizeof(uint8_t))), prismFree};
    memcpy(stable.get(), nameView.data(), nameSize);

    pm_symbol_node_t *symbolNode = allocateNode<pm_symbol_node_t>();

    pm_location_t location = parser.convertLocOffsets(nameLoc.copyWithZeroLength());

    pm_string_t unescapedString;
    // Mark string as owned so it'll be freed when the node is destroyed
    pm_string_owned_init(&unescapedString, stable.release(), nameSize);

    *symbolNode = (pm_symbol_node_t){.base = initializeBaseNode(PM_SYMBOL_NODE, location),
                                     .opening_loc = location,
                                     .value_loc = location,
                                     .closing_loc = location,
                                     .unescaped = unescapedString};

    return up_cast(symbolNode);
}

pm_node_t *Factory::AssocNode(core::LocOffsets loc, pm_node_t *key, pm_node_t *value) const {
    ENFORCE(key && value, "Key or value is null");

    pm_assoc_node_t *assocNode = allocateNode<pm_assoc_node_t>();

    pm_location_t location = parser.convertLocOffsets(loc.copyWithZeroLength());

    *assocNode = (pm_assoc_node_t){
        .base = initializeBaseNode(PM_ASSOC_NODE, location), .key = key, .value = value, .operator_loc = location};

    return up_cast(assocNode);
}

pm_node_t *Factory::Hash(core::LocOffsets loc, const vector<pm_node_t *> &pairs) const {
    pm_hash_node_t *hashNode = allocateNode<pm_hash_node_t>();

    pm_node_list_t elements = copyNodesToList(pairs);

    pm_location_t baseLoc = parser.convertLocOffsets(loc);
    pm_location_t openingLoc = {.start = nullptr, .end = nullptr};
    pm_location_t closingLoc = {.start = nullptr, .end = nullptr};

    *hashNode = (pm_hash_node_t){.base = initializeBaseNode(PM_HASH_NODE, baseLoc),
                                 .opening_loc = openingLoc,
                                 .elements = elements,
                                 .closing_loc = closingLoc};

    return up_cast(hashNode);
}

pm_node_t *Factory::KeywordHash(core::LocOffsets loc, const vector<pm_node_t *> &pairs) const {
    pm_keyword_hash_node_t *hashNode = allocateNode<pm_keyword_hash_node_t>();

    pm_node_list_t elements = copyNodesToList(pairs);

    pm_location_t baseLoc = parser.convertLocOffsets(loc);

    *hashNode =
        (pm_keyword_hash_node_t){.base = initializeBaseNode(PM_KEYWORD_HASH_NODE, baseLoc), .elements = elements};

    return up_cast(hashNode);
}

pm_node_t *Factory::SorbetPrivateStatic(core::LocOffsets loc) const {
    // Build a root-anchored constant path ::Sorbet::Private::Static
    pm_node_t *sorbet = ConstantPathNode(loc, nullptr, "Sorbet"sv);
    pm_node_t *sorbetPrivate = ConstantPathNode(loc, sorbet, "Private"sv);
    return ConstantPathNode(loc, sorbetPrivate, "Static"sv);
}

pm_node_t *Factory::TSigWithoutRuntime(core::LocOffsets loc) const {
    // Build a root-anchored constant path ::T::Sig::WithoutRuntime
    pm_node_t *tConst = ConstantPathNode(loc, nullptr, "T"sv);
    pm_node_t *tSig = ConstantPathNode(loc, tConst, "Sig"sv);
    return ConstantPathNode(loc, tSig, "WithoutRuntime"sv);
}

pm_node_t *Factory::Symbol(core::LocOffsets nameLoc, string_view name) const {
    ENFORCE(!name.empty(), "Name is empty");

    pm_constant_id_t nameId = addConstantToPool(name);
    return SymbolFromConstant(nameLoc, nameId);
}

pm_node_t *Factory::Send0(core::LocOffsets loc, pm_node_t *receiver, string_view method) const {
    ENFORCE(receiver && !method.empty(), "Receiver or method is null");

    pm_constant_id_t methodId = addConstantToPool(method);
    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, methodId, nullptr, tinyLoc, fullLoc, tinyLoc));
}

pm_node_t *Factory::Send1(core::LocOffsets loc, pm_node_t *receiver, string_view method, pm_node_t *arg1) const {
    ENFORCE(receiver && !method.empty() && arg1, "Receiver or method or argument is null");

    pm_constant_id_t methodId = addConstantToPool(method);
    pm_node_t *arguments = SingleArgumentNode(arg1);

    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, methodId, arguments, tinyLoc, fullLoc, tinyLoc));
}

pm_node_t *Factory::Send(core::LocOffsets loc, pm_node_t *receiver, string_view method, const vector<pm_node_t *> &args,
                         pm_node_t *block) const {
    ENFORCE(receiver && !method.empty(), "Receiver or method is null");

    pm_constant_id_t methodId = addConstantToPool(method);
    pm_arguments_node_t *arguments = nullptr;
    if (!args.empty()) {
        pm_location_t pmLoc = parser.convertLocOffsets(loc);
        arguments = createArgumentsNode(args, pmLoc);
    }

    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, methodId, up_cast(arguments), tinyLoc, fullLoc, tinyLoc, block));
}

pm_node_t *Factory::T(core::LocOffsets loc) const {
    return ConstantPathNode(loc, nullptr, "T");
}

pm_node_t *Factory::THelpers(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Helpers");
}

pm_node_t *Factory::TUntyped(core::LocOffsets loc) const {
    return Send0(loc, T(loc), "untyped"sv);
}

pm_node_t *Factory::TNilable(core::LocOffsets loc, pm_node_t *type) const {
    ENFORCE(type, "TNilable: type parameter is required");
    return Send1(loc, T(loc), "nilable"sv, type);
}

pm_node_t *Factory::TAny(core::LocOffsets loc, const vector<pm_node_t *> &args) const {
    ENFORCE(!args.empty(), "Args is empty");

    pm_constant_id_t methodId = addConstantToPool("any"sv);
    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());
    pm_arguments_node_t *arguments = createArgumentsNode(args, fullLoc);

    return up_cast(createSendNode(T(loc), methodId, up_cast(arguments), tinyLoc, fullLoc, tinyLoc));
}

pm_node_t *Factory::TAll(core::LocOffsets loc, const vector<pm_node_t *> &args) const {
    ENFORCE(!args.empty(), "Args is empty");

    pm_constant_id_t methodId = addConstantToPool("all"sv);
    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());
    pm_arguments_node_t *arguments = createArgumentsNode(args, fullLoc);

    return up_cast(createSendNode(T(loc), methodId, up_cast(arguments), tinyLoc, fullLoc, tinyLoc));
}

pm_node_t *Factory::TTypeParameter(core::LocOffsets loc, pm_node_t *name) const {
    ENFORCE(name, "Name is null");

    return Send1(loc, T(loc), "type_parameter"sv, name);
}

pm_node_t *Factory::TProc(core::LocOffsets loc, pm_node_t *args, pm_node_t *returnType) const {
    ENFORCE(returnType, "Return type is null");

    pm_node_t *builder = T(loc);

    builder = Send0(loc, builder, "proc"sv);

    if (args != nullptr) {
        builder = Send1(loc, builder, "params"sv, args);
    }

    return Send1(loc, builder, "returns"sv, returnType);
}

pm_node_t *Factory::TProcVoid(core::LocOffsets loc, pm_node_t *args) const {
    pm_node_t *builder = T(loc);

    builder = Send0(loc, builder, "proc"sv);

    if (args != nullptr) {
        builder = Send1(loc, builder, "params"sv, args);
    }

    return Send0(loc, builder, "void"sv);
}

pm_node_t *Factory::Send2(core::LocOffsets loc, pm_node_t *receiver, string_view method, pm_node_t *arg1,
                          pm_node_t *arg2) const {
    ENFORCE(receiver && !method.empty() && arg1 && arg2, "Receiver or method or arguments are null");

    pm_constant_id_t methodId = addConstantToPool(method);
    vector<pm_node_t *> args = {arg1, arg2};
    pm_arguments_node_t *arguments = createArgumentsNode(args, parser.convertLocOffsets(loc));

    pm_location_t fullLoc = parser.convertLocOffsets(loc);
    pm_location_t tinyLoc = parser.convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, methodId, up_cast(arguments), tinyLoc, fullLoc, tinyLoc));
}

pm_node_t *Factory::TLet(core::LocOffsets loc, pm_node_t *value, pm_node_t *type) const {
    ENFORCE(value && type, "Value or type is null");
    return Send2(loc, T(loc), "let"sv, value, type);
}

pm_node_t *Factory::TCast(core::LocOffsets loc, pm_node_t *value, pm_node_t *type) const {
    ENFORCE(value && type, "Value or type is null");
    return Send2(loc, T(loc), "cast"sv, value, type);
}

pm_node_t *Factory::TMust(core::LocOffsets loc, pm_node_t *value) const {
    ENFORCE(value, "Value is null");
    return Send1(loc, T(loc), "must"sv, value);
}

pm_node_t *Factory::TUnsafe(core::LocOffsets loc, pm_node_t *value) const {
    ENFORCE(value, "Value is null");
    return Send1(loc, T(loc), "unsafe"sv, value);
}

pm_node_t *Factory::TAbsurd(core::LocOffsets loc, pm_node_t *value) const {
    ENFORCE(value, "Value is null");
    return Send1(loc, T(loc), "absurd"sv, value);
}
pm_node_t *Factory::TBindSelf(core::LocOffsets loc, pm_node_t *type) const {
    ENFORCE(type, "Type is null");

    return Send2(loc, T(loc), "bind"sv, Self(loc), type);
}

pm_node_t *Factory::TTypeAlias(core::LocOffsets loc, pm_node_t *type) const {
    ENFORCE(type, "Type is null");

    pm_node_t *send = Send0(loc, T(loc), "type_alias"sv);

    pm_statements_node_t *stmts = allocateNode<pm_statements_node_t>();
    *stmts = (pm_statements_node_t){.base = initializeBaseNode(PM_STATEMENTS_NODE, parser.convertLocOffsets(loc)),
                                    .body = {.size = 0, .capacity = 0, .nodes = nullptr}};
    pm_node_list_append(&stmts->body, type);

    pm_node_t *block = Block(loc, up_cast(stmts));

    auto *call = down_cast<pm_call_node_t>(send);
    call->block = block;

    return send;
}

pm_node_t *Factory::Array(core::LocOffsets loc, const vector<pm_node_t *> &elements) const {
    pm_array_node_t *array = allocateNode<pm_array_node_t>();

    pm_node_list_t elemNodes = copyNodesToList(elements);

    *array = (pm_array_node_t){.base = initializeBaseNode(PM_ARRAY_NODE, parser.convertLocOffsets(loc)),
                               .elements = elemNodes,
                               .opening_loc = parser.convertLocOffsets(loc.copyWithZeroLength()),
                               .closing_loc = parser.convertLocOffsets(loc.copyEndWithZeroLength())};

    return up_cast(array);
}

pm_node_t *Factory::Block(core::LocOffsets loc, pm_node_t *body) const {
    pm_block_node_t *block = allocateNode<pm_block_node_t>();

    pm_location_t zeroLoc = parser.getZeroWidthLocation();

    *block = (pm_block_node_t){.base = initializeBaseNode(PM_BLOCK_NODE, parser.convertLocOffsets(loc)),
                               .locals = {.size = 0, .capacity = 0, .ids = nullptr},
                               .parameters = nullptr,
                               .body = body,
                               .opening_loc = zeroLoc,
                               .closing_loc = zeroLoc};

    return up_cast(block);
}

pm_node_t *Factory::T_Array(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Array"sv);
}

pm_node_t *Factory::T_Class(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Class"sv);
}

pm_node_t *Factory::T_Enumerable(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Enumerable"sv);
}

pm_node_t *Factory::T_Enumerator(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Enumerator"sv);
}

pm_node_t *Factory::T_Hash(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Hash"sv);
}

pm_node_t *Factory::T_Set(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Set"sv);
}

pm_node_t *Factory::T_Range(core::LocOffsets loc) const {
    return ConstantPathNode(loc, T(loc), "Range"sv);
}

pm_node_t *Factory::StatementsNode(core::LocOffsets loc, const vector<pm_node_t *> &body) const {
    pm_statements_node_t *stmts = allocateNode<pm_statements_node_t>();
    *stmts = (pm_statements_node_t){.base = initializeBaseNode(PM_STATEMENTS_NODE, parser.convertLocOffsets(loc)),
                                    .body = {.size = 0, .capacity = 0, .nodes = nullptr}};

    for (auto *node : body) {
        pm_node_list_append(&stmts->body, node);
    }

    return up_cast(stmts);
}

void *Factory::malloc(size_t size) const {
    void *p = ::xmalloc(size); // see Prism's `include/prism/defines.h`
    if (!p) {
        throw std::bad_alloc{};
    };
    return p;
}

void *Factory::calloc(size_t count, size_t size) const {
    void *p = ::xcalloc(count, size); // see Prism's `include/prism/defines.h`
    if (!p) {
        throw std::bad_alloc{};
    };
    return p;
}

void *Factory::realloc(void *ptr, size_t size) const {
    void *p = ::xrealloc(ptr, size); // see Prism's `include/prism/defines.h`
    if (!p) {
        throw std::bad_alloc{};
    };
    return p;
}

void Factory::free(void *ptr) const { // see Prism's `include/prism/defines.h`
    ENFORCE(ptr);
    ::xfree(ptr);
}

} // namespace sorbet::parser::Prism
