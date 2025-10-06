#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::parser::Prism {

// Global parser instance for node creation
static thread_local const Parser *g_prismParser = nullptr;

void PMK::setParser(const Parser *parser) {
    g_prismParser = parser;
}

pm_node_t PMK::initializeBaseNode(pm_node_type_t type) {
    if (!g_prismParser) {
        return (pm_node_t){.type = type, .flags = 0, .node_id = 0, .location = {.start = nullptr, .end = nullptr}};
    }

    pm_parser_t *p = g_prismParser->getInternalParser();
    pm_location_t loc = getZeroWidthLocation();

    return (pm_node_t){.type = type, .flags = 0, .node_id = ++p->node_id, .location = loc};
}

pm_node_t *PMK::ConstantReadNode(const char *name) {
    pm_constant_id_t constant_id = addConstantToPool(name);
    if (constant_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_read_node_t *node = allocateNode<pm_constant_read_node_t>();
    if (!node)
        return nullptr;

    *node = (pm_constant_read_node_t){.base = initializeBaseNode(PM_CONSTANT_READ_NODE), .name = constant_id};

    return up_cast(node);
}

pm_node_t *PMK::ConstantPathNode(core::LocOffsets loc, pm_node_t *parent, const char *name) {
    pm_constant_id_t name_id = addConstantToPool(name);
    if (name_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_path_node_t *node = allocateNode<pm_constant_path_node_t>();
    if (!node)
        return nullptr;

    pm_location_t pm_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = getZeroWidthLocation();

    *node = (pm_constant_path_node_t){.base = initializeBaseNode(PM_CONSTANT_PATH_NODE),
                                      .parent = parent,
                                      .name = name_id,
                                      .delimiter_loc = tiny_loc,
                                      .name_loc = pm_loc};

    return up_cast(node);
}

pm_node_t *PMK::SingleArgumentNode(pm_node_t *arg) {
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments)
        return nullptr;

    pm_node_t **arg_nodes = (pm_node_t **)calloc(1, sizeof(pm_node_t *));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }
    arg_nodes[0] = arg;

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE),
                                       .arguments = {.size = 1, .capacity = 1, .nodes = arg_nodes}};
    arguments->base.location = arg->location;

    return up_cast(arguments);
}

pm_node_t *PMK::Self(core::LocOffsets loc) {
    pm_self_node_t *selfNode = allocateNode<pm_self_node_t>();
    if (!selfNode)
        return nullptr;

    *selfNode = (pm_self_node_t){.base = initializeBaseNode(PM_SELF_NODE)};
    if (loc != core::LocOffsets::none()) {
        selfNode->base.location = convertLocOffsets(loc);
    }

    return up_cast(selfNode);
}

pm_constant_id_t PMK::addConstantToPool(const char *name) {
    if (!g_prismParser)
        return PM_CONSTANT_ID_UNSET;

    pm_parser_t *p = g_prismParser->getInternalParser();
    size_t name_len = strlen(name);
    uint8_t *stable = (uint8_t *)calloc(name_len, sizeof(uint8_t));
    if (!stable) {
        return PM_CONSTANT_ID_UNSET;
    }
    memcpy(stable, name, name_len);
    pm_constant_id_t id = pm_constant_pool_insert_constant(&p->constant_pool, stable, name_len);
    return id;
}

pm_location_t PMK::getZeroWidthLocation() {
    if (!g_prismParser) {
        return {.start = nullptr, .end = nullptr};
    }

    pm_parser_t *p = g_prismParser->getInternalParser();
    const uint8_t *source_start = p->start;
    return {.start = source_start, .end = source_start};
}

pm_location_t PMK::convertLocOffsets(core::LocOffsets loc) {
    if (!g_prismParser) {
        return {.start = nullptr, .end = nullptr};
    }

    pm_parser_t *p = g_prismParser->getInternalParser();
    const uint8_t *source_start = p->start;

    const uint8_t *start_ptr = source_start + loc.beginPos();
    const uint8_t *end_ptr = source_start + loc.endPos();

    return {.start = start_ptr, .end = end_ptr};
}

pm_call_node_t *PMK::createSendNode(pm_node_t *receiver, pm_constant_id_t method_id, pm_node_t *arguments,
                                    pm_location_t message_loc, pm_location_t full_loc, pm_location_t tiny_loc,
                                    pm_node_t *block) {
    pm_call_node_t *call = allocateNode<pm_call_node_t>();
    if (!call) {
        return nullptr;
    }

    *call = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                             .receiver = receiver,
                             .call_operator_loc = tiny_loc,
                             .name = method_id,
                             .message_loc = message_loc,
                             .opening_loc = tiny_loc,
                             .arguments = down_cast<pm_arguments_node_t>(arguments),
                             .closing_loc = tiny_loc,
                             .block = block};
    call->base.location = full_loc;

    return call;
}

pm_node_t *PMK::SymbolFromConstant(core::LocOffsets nameLoc, pm_constant_id_t nameId) {
    if (!g_prismParser || nameId == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    auto nameView = g_prismParser->resolveConstant(nameId);
    string nameString(nameView);

    uint8_t *stable = (uint8_t *)calloc(nameString.size(), sizeof(uint8_t));
    if (!stable) {
        return nullptr;
    }
    memcpy(stable, nameString.data(), nameString.size());

    pm_symbol_node_t *symbolNode = allocateNode<pm_symbol_node_t>();
    if (!symbolNode) {
        return nullptr;
    }

    pm_location_t location = convertLocOffsets(nameLoc.copyWithZeroLength());

    pm_string_t unescaped_string;
    unescaped_string.source = stable;
    unescaped_string.length = nameString.length();

    *symbolNode = (pm_symbol_node_t){.base = initializeBaseNode(PM_SYMBOL_NODE),
                                     .opening_loc = location,
                                     .value_loc = location,
                                     .closing_loc = location,
                                     .unescaped = unescaped_string};
    symbolNode->base.location = location;

    return up_cast(symbolNode);
}

pm_node_t *PMK::AssocNode(core::LocOffsets loc, pm_node_t *key, pm_node_t *value) {
    if (!key || !value) {
        return nullptr;
    }

    pm_assoc_node_t *assocNode = allocateNode<pm_assoc_node_t>();
    if (!assocNode) {
        return nullptr;
    }

    pm_location_t location = convertLocOffsets(loc.copyWithZeroLength());

    *assocNode = (pm_assoc_node_t){
        .base = initializeBaseNode(PM_ASSOC_NODE), .key = key, .value = value, .operator_loc = location};
    assocNode->base.location = location;

    return up_cast(assocNode);
}

pm_node_t *PMK::Hash(core::LocOffsets loc, const vector<pm_node_t *> &pairs) {
    if (pairs.empty()) {
        return nullptr;
    }

    pm_hash_node_t *hashNode = allocateNode<pm_hash_node_t>();
    if (!hashNode) {
        return nullptr;
    }

    pm_node_t **elements = nullptr;
    if (!pairs.empty()) {
        elements = (pm_node_t **)calloc(pairs.size(), sizeof(pm_node_t *));
        if (!elements) {
            return nullptr;
        }
        for (size_t i = 0; i < pairs.size(); i++) {
            elements[i] = pairs[i];
        }
    }

    pm_location_t base_loc = convertLocOffsets(loc);
    pm_location_t opening_loc = {.start = nullptr, .end = nullptr};
    pm_location_t closing_loc = {.start = nullptr, .end = nullptr};

    *hashNode = (pm_hash_node_t){.base = initializeBaseNode(PM_HASH_NODE),
                                 .opening_loc = opening_loc,
                                 .elements = {.size = pairs.size(), .capacity = pairs.size(), .nodes = elements},
                                 .closing_loc = closing_loc};
    hashNode->base.location = base_loc;

    return up_cast(hashNode);
}

pm_node_t *PMK::KeywordHash(core::LocOffsets loc, const vector<pm_node_t *> &pairs) {
    if (pairs.empty()) {
        return nullptr;
    }

    pm_keyword_hash_node_t *hashNode = allocateNode<pm_keyword_hash_node_t>();
    if (!hashNode) {
        return nullptr;
    }

    pm_node_t **elements = nullptr;
    elements = (pm_node_t **)calloc(pairs.size(), sizeof(pm_node_t *));
    if (!elements) {
        return nullptr;
    }
    for (size_t i = 0; i < pairs.size(); i++) {
        elements[i] = pairs[i];
    }

    pm_location_t base_loc = convertLocOffsets(loc);

    *hashNode =
        (pm_keyword_hash_node_t){.base = initializeBaseNode(PM_KEYWORD_HASH_NODE),
                                 .elements = {.size = pairs.size(), .capacity = pairs.size(), .nodes = elements}};
    hashNode->base.location = base_loc;

    return up_cast(hashNode);
}

void PMK::debugPrintLocation(const char *label, pm_location_t loc) {
    (void)label;
    (void)loc;
    // Debug implementation commented out for performance
}

pm_node_t *PMK::SorbetPrivateStatic() {
    // Build a root-anchored constant path ::Sorbet::Private::Static
    pm_node_t *sorbet = ConstantPathNode(core::LocOffsets::none(), nullptr, "Sorbet");
    if (!sorbet)
        return nullptr;

    pm_node_t *sorbet_private = ConstantPathNode(core::LocOffsets::none(), sorbet, "Private");
    if (!sorbet_private)
        return nullptr;

    return ConstantPathNode(core::LocOffsets::none(), sorbet_private, "Static");
}

pm_node_t *PMK::TSigWithoutRuntime() {
    // Build a root-anchored constant path ::T::Sig::WithoutRuntime
    pm_node_t *t_const = ConstantPathNode(core::LocOffsets::none(), nullptr, "T");
    if (!t_const)
        return nullptr;

    pm_node_t *t_sig = ConstantPathNode(core::LocOffsets::none(), t_const, "Sig");
    if (!t_sig)
        return nullptr;

    return ConstantPathNode(core::LocOffsets::none(), t_sig, "WithoutRuntime");
}

pm_node_t *PMK::Symbol(core::LocOffsets nameLoc, const char *name) {
    if (!name) {
        return nullptr;
    }

    pm_constant_id_t nameId = addConstantToPool(name);
    return SymbolFromConstant(nameLoc, nameId);
}

pm_node_t *PMK::Send0(core::LocOffsets loc, pm_node_t *receiver, const char *method) {
    if (!receiver || !method) {
        return nullptr;
    }

    pm_constant_id_t method_id = addConstantToPool(method);
    if (method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    pm_location_t full_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, method_id, nullptr, tiny_loc, full_loc, tiny_loc));
}

pm_node_t *PMK::Send1(core::LocOffsets loc, pm_node_t *receiver, const char *method, pm_node_t *arg1) {
    if (!receiver || !method || !arg1) {
        return nullptr;
    }

    pm_constant_id_t method_id = addConstantToPool(method);
    if (method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    pm_node_t *arguments = SingleArgumentNode(arg1);
    if (!arguments) {
        return nullptr;
    }

    pm_location_t full_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, method_id, arguments, tiny_loc, full_loc, tiny_loc));
}

pm_node_t *PMK::Send(core::LocOffsets loc, pm_node_t *receiver, const char *method,
                     const vector<pm_node_t *> &args, pm_node_t *block) {
    if (!receiver || !method || args.empty()) {
        return nullptr;
    }

    pm_constant_id_t method_id = addConstantToPool(method);
    if (method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    // Create arguments node with multiple arguments
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments) {
        return nullptr;
    }

    pm_node_t **arg_nodes = (pm_node_t **)calloc(args.size(), sizeof(pm_node_t *));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }

    for (size_t i = 0; i < args.size(); i++) {
        arg_nodes[i] = args[i];
    }

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE),
                                       .arguments = {.size = args.size(), .capacity = args.size(), .nodes = arg_nodes}};
    arguments->base.location = convertLocOffsets(loc);

    pm_location_t full_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(receiver, method_id, up_cast(arguments), tiny_loc, full_loc, tiny_loc, block));
}

pm_node_t *PMK::T(core::LocOffsets loc) {
    // Create ::T constant path node
    return ConstantPathNode(loc, nullptr, "T");
}

pm_node_t *PMK::TUntyped(core::LocOffsets loc) {
    // Create T.untyped call
    pm_node_t *t_const = T(loc);
    if (!t_const) {
        return nullptr;
    }

    return Send0(loc, t_const, "untyped");
}

pm_node_t *PMK::TNilable(core::LocOffsets loc, pm_node_t *type) {
    // Create T.nilable(type) call
    pm_node_t *t_const = T(loc);
    if (!t_const || !type) {
        return nullptr;
    }

    return Send1(loc, t_const, "nilable", type);
}

pm_node_t *PMK::TAny(core::LocOffsets loc, const vector<pm_node_t *> &args) {
    // Create T.any(args...) call
    pm_node_t *t_const = T(loc);
    if (!t_const || args.empty()) {
        return nullptr;
    }

    // Create arguments node with multiple arguments
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments) {
        return nullptr;
    }

    pm_node_t **arg_nodes = (pm_node_t **)calloc(args.size(), sizeof(pm_node_t *));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }

    for (size_t i = 0; i < args.size(); i++) {
        arg_nodes[i] = args[i];
    }

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE),
                                       .arguments = {.size = args.size(), .capacity = args.size(), .nodes = arg_nodes}};
    arguments->base.location = convertLocOffsets(loc);

    pm_constant_id_t method_id = addConstantToPool("any");
    if (method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    pm_location_t full_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(t_const, method_id, up_cast(arguments), tiny_loc, full_loc, tiny_loc));
}

pm_node_t *PMK::TAll(core::LocOffsets loc, const vector<pm_node_t *> &args) {
    // Create T.all(args...) call
    pm_node_t *t_const = T(loc);
    if (!t_const || args.empty()) {
        return nullptr;
    }

    // Create arguments node with multiple arguments
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments) {
        return nullptr;
    }

    pm_node_t **arg_nodes = (pm_node_t **)calloc(args.size(), sizeof(pm_node_t *));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }

    for (size_t i = 0; i < args.size(); i++) {
        arg_nodes[i] = args[i];
    }

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE),
                                       .arguments = {.size = args.size(), .capacity = args.size(), .nodes = arg_nodes}};
    arguments->base.location = convertLocOffsets(loc);

    pm_constant_id_t method_id = addConstantToPool("all");
    if (method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    pm_location_t full_loc = convertLocOffsets(loc);
    pm_location_t tiny_loc = convertLocOffsets(loc.copyWithZeroLength());

    return up_cast(createSendNode(t_const, method_id, up_cast(arguments), tiny_loc, full_loc, tiny_loc));
}

pm_node_t *PMK::TTypeParameter(core::LocOffsets loc, pm_node_t *name) {
    // Create T.type_parameter(name) call
    pm_node_t *t_const = T(loc);
    if (!t_const || !name) {
        return nullptr;
    }

    return Send1(loc, t_const, "type_parameter", name);
}

pm_node_t *PMK::TProc(core::LocOffsets loc, pm_node_t *args, pm_node_t *returnType) {
    // Create T.proc.params(args).returns(returnType) call
    pm_node_t *builder = T(loc);
    if (!builder || !returnType) {
        return nullptr;
    }

    builder = Send0(loc, builder, "proc");
    if (!builder) {
        return nullptr;
    }

    if (args != nullptr) {
        builder = Send1(loc, builder, "params", args);
        if (!builder) {
            return nullptr;
        }
    }

    return Send1(loc, builder, "returns", returnType);
}

pm_node_t *PMK::TProcVoid(core::LocOffsets loc, pm_node_t *args) {
    // Create T.proc.params(args).void call
    pm_node_t *builder = T(loc);
    if (!builder) {
        return nullptr;
    }

    builder = Send0(loc, builder, "proc");
    if (!builder) {
        return nullptr;
    }

    if (args != nullptr) {
        builder = Send1(loc, builder, "params", args);
        if (!builder) {
            return nullptr;
        }
    }

    return Send0(loc, builder, "void");
}

bool PMK::isTUntyped(pm_node_t *node) {
    if (!node || node->type != PM_CALL_NODE) {
        return false;
    }

    pm_call_node_t *call = down_cast<pm_call_node_t>(node);
    if (!call->receiver || call->receiver->type != PM_CONSTANT_PATH_NODE) {
        return false;
    }

    // Check if receiver is ::T and method is "untyped"
    pm_constant_path_node_t *receiver = down_cast<pm_constant_path_node_t>(call->receiver);
    if (receiver->parent != nullptr) {
        return false; // Should be root-anchored ::T
    }

    // Check method name is "untyped" (this is simplified - would need constant pool lookup)
    // For now, return false as a safe fallback
    return false;
}

} // namespace sorbet::parser::Prism
