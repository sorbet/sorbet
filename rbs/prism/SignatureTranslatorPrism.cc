#include "rbs/prism/SignatureTranslatorPrism.h"
#include "core/errors/rewriter.h"
#include "parser/prism/Helpers.h"
#include "rbs/MethodTypeToParserNode.h"
#include "rbs/TypeParamsToParserNodes.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/prism/MethodTypeToParserNodePrism.h"
#include "rbs/rbs_common.h"
#include <cstring>

extern "C" {
#include "prism/util/pm_constant_pool.h"
}

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

rbs_string_t makeRBSString(const string &str) {
    return rbs_string_new(str.data(), str.data() + str.size());
}

} // namespace

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateAssertionType(vector<pair<core::LocOffsets,
// core::NameRef>> typeParams,
//                                                  const rbs::RBSDeclaration &assertion) {
//     rbs_string_t rbsString = makeRBSString(assertion.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets loc = assertion.typeLocFromRange(parser.getError()->token.range);
//         if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
//         }
//         return nullptr;
//     }

//     auto typeToParserNode = TypeToParserNode(ctx, typeParams, move(parser));
//     return typeToParserNode.toParserNode(rbsType, assertion);
// }

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateAttrSignature(const pm_call_node_t *call,
//                                                                           const RBSDeclaration &declaration,
//                                                                           const vector<Comment> &annotations) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets offset = declaration.typeLocFromRange(parser.getError()->token.range);
//         // First parse failed, let's check if the user mistakenly used a method signature on an accessor
//         auto methodParser = Parser(rbsString, encoding);
//         methodParser.parseMethodType();

//         if (!methodParser.hasError()) {
//             if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//                 e.setHeader("Using a method signature on an accessor is not allowed, use a bare type instead");
//             }
//         } else {
//             if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//                 e.setHeader("Failed to parse RBS type ({})", methodParser.getError()->message);
//             }
//         }

//         return nullptr;
//     }

//     // TODO: Need to implement MethodTypeToParserNode.attrSignature to work with Prism nodes
//     // For now, we'll need to convert pm_call_node_t to parser::Send for compatibility
//     (void)call; // Suppress unused variable warning
//     (void)rbsType; // Suppress unused variable warning
//     (void)annotations; // Suppress unused variable warning

//     // Temporary stub - needs proper implementation
//     // auto methodTypeToParserNode = MethodTypeToParserNode(ctx, move(parser));
//     // return methodTypeToParserNode.attrSignature(prismCallToParserSend(call), rbsType, declaration, annotations);
//     return nullptr;
// }

unique_ptr<parser::Node> SignatureTranslatorPrism::translateMethodSignature(const pm_node_t *methodDef,
                                                                            const RBSDeclaration &declaration,
                                                                            const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_method_type_t *rbsMethodType = parser.parseMethodType();

    if (parser.hasError()) {
        rbs_range_t tokenRange = parser.getError()->token.range;
        core::LocOffsets offset = declaration.typeLocFromRange(tokenRange);

        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser.getError()->message);
        }

        return nullptr;
    }

    auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism(ctx, move(parser));
    return methodTypeToParserNodePrism.methodSignature(methodDef, rbsMethodType, declaration, annotations);
}

// New Prism node creation methods
pm_node_t* SignatureTranslatorPrism::createPrismMethodSignature(const pm_node_t *methodDef,
                                                                const RBSDeclaration &declaration,
                                                                const std::vector<Comment> &annotations) {
    if (!parser) {
        return nullptr; // Need parser for Prism node creation
    }

    // For now, create a placeholder sig call
    // TODO: Parse the RBS declaration and create appropriate signature nodes
    (void)methodDef;    // Suppress unused warnings for now
    (void)declaration;
    (void)annotations;

    return createSigCallPlaceholder();
}

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateType(const RBSDeclaration &declaration) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets offset = declaration.typeLocFromRange(parser.getError()->token.range);
//         if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
//         }

//         return nullptr;
//     }

//     auto typeTranslator = TypeToParserNode(ctx, vector<pair<core::LocOffsets, core::NameRef>>(), parser);
//     return typeTranslator.toParserNode(rbsType, declaration);
// }

// parser::NodeVec SignatureTranslatorPrism::translateTypeParams(const RBSDeclaration &declaration) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_list_t *rbsTypeParams = parser.parseTypeParams();

//     if (parser.hasError()) {
//         rbs_range_t tokenRange = parser.getError()->token.range;
//         core::LocOffsets offset = declaration.typeLocFromRange(tokenRange);

//         if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type parameters ({})", parser.getError()->message);
//         }

//         return parser::NodeVec();
//     }

//     auto typeParamsToParserNode = TypeParamsToParserNode(ctx, move(parser));
//     return typeParamsToParserNode.typeParams(rbsTypeParams, declaration);
// }

// Node creation helper implementations (moved from SigsRewriterPrism)

template<typename T>
T* SignatureTranslatorPrism::allocateNode() {
    T* node = (T*) calloc(1, sizeof(T));
    return node; // Returns nullptr on allocation failure
}

pm_node_t SignatureTranslatorPrism::initializeBaseNode(pm_node_type_t type) {
    if (!parser) {
        // Return a default-initialized node if parser is not available
        return (pm_node_t) { .type = type, .flags = 0, .node_id = 0, .location = { .start = nullptr, .end = nullptr } };
    }

    pm_parser_t* p = parser->getInternalParser();
    pm_location_t loc = getZeroWidthLocation();

    return (pm_node_t) {
        .type = type,
        .flags = 0,
        .node_id = ++p->node_id,
        .location = loc
    };
}

pm_node_t* SignatureTranslatorPrism::createConstantReadNode(const char* name) {
    pm_constant_id_t constant_id = addConstantToPool(name);
    if (constant_id == PM_CONSTANT_ID_UNSET) return nullptr;

    pm_constant_read_node_t* node = allocateNode<pm_constant_read_node_t>();
    if (!node) return nullptr;

    *node = (pm_constant_read_node_t) {
        .base = initializeBaseNode(PM_CONSTANT_READ_NODE),
        .name = constant_id
    };

    return up_cast(node);
}

pm_node_t* SignatureTranslatorPrism::createConstantPathNode(pm_node_t* parent, const char* name) {
    pm_constant_id_t name_id = addConstantToPool(name);
    if (name_id == PM_CONSTANT_ID_UNSET) return nullptr;

    pm_constant_path_node_t* node = allocateNode<pm_constant_path_node_t>();
    if (!node) return nullptr;

    pm_location_t loc = getZeroWidthLocation();

    *node = (pm_constant_path_node_t) {
        .base = initializeBaseNode(PM_CONSTANT_PATH_NODE),
        .parent = parent,
        .name = name_id,
        .delimiter_loc = loc,
        .name_loc = loc
    };

    return up_cast(node);
}

pm_node_t* SignatureTranslatorPrism::createSingleArgumentNode(pm_node_t* arg) {
    pm_arguments_node_t* arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments) return nullptr;

    pm_node_t** arg_nodes = (pm_node_t**) calloc(1, sizeof(pm_node_t*));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }
    arg_nodes[0] = arg;

    *arguments = (pm_arguments_node_t) {
        .base = initializeBaseNode(PM_ARGUMENTS_NODE),
        .arguments = { .size = 1, .capacity = 1, .nodes = arg_nodes }
    };

    return up_cast(arguments);
}

pm_node_t* SignatureTranslatorPrism::createSorbetPrivateStaticConstant() {
    // Create Sorbet constant read
    pm_node_t* sorbet = createConstantReadNode("Sorbet");
    if (!sorbet) return nullptr;

    // Create Sorbet::Private constant path
    pm_node_t* sorbet_private = createConstantPathNode(sorbet, "Private");
    if (!sorbet_private) return nullptr;

    // Create Sorbet::Private::Static constant path
    return createConstantPathNode(sorbet_private, "Static");
}

pm_node_t* SignatureTranslatorPrism::createTSigWithoutRuntimeConstant() {
    // Create T constant read
    pm_node_t* t_const = createConstantReadNode("T");
    if (!t_const) return nullptr;

    // Create T::Sig constant path
    pm_node_t* t_sig = createConstantPathNode(t_const, "Sig");
    if (!t_sig) return nullptr;

    // Create T::Sig::WithoutRuntime constant path
    return createConstantPathNode(t_sig, "WithoutRuntime");
}

pm_node_t* SignatureTranslatorPrism::createStringConstant() {
    return createConstantReadNode("String");
}

pm_node_t* SignatureTranslatorPrism::createSigCallPlaceholder() {
    if (!parser) return nullptr;

    pm_location_t loc = getZeroWidthLocation();

    // Add method names to constant pool
    pm_constant_id_t sig_method_id = addConstantToPool("sig");
    pm_constant_id_t returns_id = addConstantToPool("returns");

    if (sig_method_id == PM_CONSTANT_ID_UNSET || returns_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    // Create receiver: Sorbet::Private::Static
    pm_node_t* receiver = createSorbetPrivateStaticConstant();
    if (!receiver) return nullptr;

    // Create argument: T::Sig::WithoutRuntime
    pm_node_t* t_sig_arg = createTSigWithoutRuntimeConstant();
    if (!t_sig_arg) return nullptr;

    // Create arguments list
    pm_node_t* arguments = createSingleArgumentNode(t_sig_arg);
    if (!arguments) return nullptr;

    // Create block body: returns(String)
    pm_node_t* string_const = createStringConstant();
    if (!string_const) return nullptr;

    // Create arguments for returns call
    pm_node_t* returns_arguments = createSingleArgumentNode(string_const);
    if (!returns_arguments) return nullptr;

    // Create returns(String) call
    pm_call_node_t *returns_call = allocateNode<pm_call_node_t>();
    if (!returns_call) return nullptr;

    *returns_call = (pm_call_node_t) {
        .base = initializeBaseNode(PM_CALL_NODE),
        .receiver = nullptr,        // No explicit receiver (implicit self)
        .call_operator_loc = { .start = nullptr, .end = nullptr },
        .name = returns_id,
        .message_loc = loc,
        .opening_loc = loc,
        .arguments = down_cast<pm_arguments_node_t>(returns_arguments),
        .closing_loc = loc,
        .block = nullptr
    };

    // Create block node
    pm_block_node_t *block = allocateNode<pm_block_node_t>();
    if (!block) return nullptr;

    *block = (pm_block_node_t) {
        .base = initializeBaseNode(PM_BLOCK_NODE),
        .locals = { .size = 0, .capacity = 0, .ids = nullptr },
        .parameters = nullptr,
        .body = up_cast(returns_call),
        .opening_loc = loc,
        .closing_loc = loc
    };

    // Create the main sig call: Sorbet::Private::Static.sig(T::Sig::WithoutRuntime) { returns(String) }
    pm_call_node_t *call = allocateNode<pm_call_node_t>();
    if (!call) return nullptr;

    *call = (pm_call_node_t) {
        .base = initializeBaseNode(PM_CALL_NODE),
        .receiver = receiver,       // Sorbet::Private::Static constant path
        .call_operator_loc = loc,
        .name = sig_method_id,      // "sig" method
        .message_loc = loc,
        .opening_loc = loc,
        .arguments = down_cast<pm_arguments_node_t>(arguments),  // T::Sig::WithoutRuntime argument
        .closing_loc = loc,
        .block = up_cast(block)            // { returns(String) } block
    };

    return up_cast(call);
}

pm_constant_id_t SignatureTranslatorPrism::addConstantToPool(const char* name) {
    if (!parser) return PM_CONSTANT_ID_UNSET;

    pm_parser_t* p = parser->getInternalParser();
    pm_constant_id_t id = pm_constant_pool_insert_constant(
        &p->constant_pool,
        (const uint8_t*)name,
        strlen(name)
    );
    return id; // Returns PM_CONSTANT_ID_UNSET on failure
}

pm_location_t SignatureTranslatorPrism::getZeroWidthLocation() {
    if (!parser) {
        return { .start = nullptr, .end = nullptr };
    }

    pm_parser_t* p = parser->getInternalParser();
    const uint8_t* source_start = p->start;
    return { .start = source_start, .end = source_start };
}

} // namespace sorbet::rbs
