#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"
#include <cstdlib>
#include <cstring>

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

pm_node_t *PMK::createConstantReadNode(const char *name) {
    pm_constant_id_t constant_id = addConstantToPool(name);
    if (constant_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_read_node_t *node = allocateNode<pm_constant_read_node_t>();
    if (!node)
        return nullptr;

    *node = (pm_constant_read_node_t){.base = initializeBaseNode(PM_CONSTANT_READ_NODE), .name = constant_id};

    return up_cast(node);
}

pm_node_t *PMK::createConstantPathNode(pm_node_t *parent, const char *name) {
    pm_constant_id_t name_id = addConstantToPool(name);
    if (name_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_path_node_t *node = allocateNode<pm_constant_path_node_t>();
    if (!node)
        return nullptr;

    pm_location_t loc = getZeroWidthLocation();

    *node = (pm_constant_path_node_t){.base = initializeBaseNode(PM_CONSTANT_PATH_NODE),
                                      .parent = parent,
                                      .name = name_id,
                                      .delimiter_loc = loc,
                                      .name_loc = loc};

    return up_cast(node);
}

pm_node_t *PMK::createSingleArgumentNode(pm_node_t *arg) {
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

pm_node_t *PMK::createSelfNode() {
    pm_self_node_t *selfNode = allocateNode<pm_self_node_t>();
    if (!selfNode)
        return nullptr;

    *selfNode = (pm_self_node_t){.base = initializeBaseNode(PM_SELF_NODE)};

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

pm_call_node_t *PMK::createMethodCall(pm_node_t *receiver, pm_constant_id_t method_id,
                                                       pm_node_t *arguments, pm_location_t message_loc,
                                                       pm_location_t full_loc, pm_location_t tiny_loc,
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

pm_node_t *PMK::createSymbolNodeFromConstant(pm_constant_id_t nameId, core::LocOffsets nameLoc) {
    if (!g_prismParser || nameId == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    auto nameView = g_prismParser->resolveConstant(nameId);
    std::string nameString(nameView);

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

pm_node_t *PMK::createAssocNode(pm_node_t *key, pm_node_t *value, core::LocOffsets loc) {
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

pm_node_t *PMK::createHashNode(const std::vector<pm_node_t *> &pairs, core::LocOffsets loc) {
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

pm_node_t *PMK::createKeywordHashNode(const std::vector<pm_node_t *> &pairs, core::LocOffsets loc) {
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

pm_node_t *PMK::createSorbetPrivateStaticConstant() {
    // Build a root-anchored constant path ::Sorbet::Private::Static
    pm_node_t *sorbet = createConstantPathNode(nullptr, "Sorbet");
    if (!sorbet)
        return nullptr;

    pm_node_t *sorbet_private = createConstantPathNode(sorbet, "Private");
    if (!sorbet_private)
        return nullptr;

    return createConstantPathNode(sorbet_private, "Static");
}

pm_node_t *PMK::createTSigWithoutRuntimeConstant() {
    // Build a root-anchored constant path ::T::Sig::WithoutRuntime
    pm_node_t *t_const = createConstantPathNode(nullptr, "T");
    if (!t_const)
        return nullptr;

    pm_node_t *t_sig = createConstantPathNode(t_const, "Sig");
    if (!t_sig)
        return nullptr;

    return createConstantPathNode(t_sig, "WithoutRuntime");
}

pm_node_t *PMK::createSymbolNode(const char *name, core::LocOffsets nameLoc) {
    if (!name) {
        return nullptr;
    }

    pm_constant_id_t nameId = addConstantToPool(name);
    return createSymbolNodeFromConstant(nameId, nameLoc);
}

} // namespace sorbet::parser::Prism
