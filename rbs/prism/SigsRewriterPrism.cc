#include "rbs/prism/SigsRewriterPrism.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "parser/prism/Helpers.h"
#include "rbs/SignatureTranslator.h"
#include "rbs/TypeParamsToParserNodes.h"

extern "C" {
#include "prism.h"
}
#include "rbs/prism/SignatureTranslatorPrism.h"

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

// Prism version of signaturesTarget
pm_node_t *signaturesTargetForPrism(pm_node_t *node) {
    if (node == nullptr) {
        return nullptr;
    }

    switch (PM_NODE_TYPE(node)) {
        case PM_DEF_NODE:
            // Both instance methods and singleton methods use PM_DEF_NODE in Prism
            // (singleton methods have a receiver field set)
            return node;
        case PM_CALL_NODE: {
            // TODO: Need to implement isAttrAccessorSend and isVisibilitySend for Prism nodes
            // This requires checking the method name and context
            // For now, return nullptr until we implement the helper functions
            return nullptr;
        }
        default:
            return nullptr;
    }
}

/**
 * Extracts and parses the argument from the annotation string.
 *
 * Considering an annotation like `@mixes_in_class_methods: ClassMethods`,
 * this function will extract and parse `ClassMethods` as a type then return the corresponding parser::Node.
 *
 * We do not error if the node is not a constant, we just insert it as is and let the pipeline error down the line.
 */
// unique_ptr<parser::Node> extractHelperArgumentPrism(core::MutableContext ctx, Comment annotation, int offset) {
//     while (annotation.string[offset] == ' ') {
//         offset++;
//     }

//     Comment comment = {
//         core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
//         core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
//         annotation.string.substr(offset),
//     };

//     return rbs::SignatureTranslator(ctx).translateType(RBSDeclaration{vector<Comment>{comment}});
// }

// TODO(Prism migration): helpers extraction is not required for signature translation; keeping for reference.
// parser::NodeVec extractHelpersPrism(core::MutableContext ctx, vector<Comment> annotations) {
//     parser::NodeVec helpers;
//     for (auto &annotation : annotations) {
//         if (annotation.string == "abstract") {
//             auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
//                                           core::Names::declareAbstract(), annotation.typeLoc);
//             helpers.emplace_back(move(send));
//         } else if (annotation.string == "interface") {
//             auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
//                                           core::Names::declareInterface(), annotation.typeLoc);
//             helpers.emplace_back(move(send));
//         } else if (annotation.string == "final") {
//             auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
//                                           core::Names::declareFinal(), annotation.typeLoc);
//             helpers.emplace_back(move(send));
//         } else if (annotation.string == "sealed") {
//             auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
//                                           core::Names::declareSealed(), annotation.typeLoc);
//             helpers.emplace_back(move(send));
//         } else if (absl::StartsWith(annotation.string, "requires_ancestor:")) {
//             if (auto type = extractHelperArgumentPrism(ctx, annotation, 18)) {
//                 auto body = make_unique<parser::Begin>(annotation.typeLoc, parser::NodeVec());
//                 body->stmts.emplace_back(move(type));
//                 auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
//                                               core::Names::requiresAncestor(), annotation.typeLoc);
//                 auto block = make_unique<parser::Block>(annotation.typeLoc, move(send), nullptr, move(body));
//                 helpers.emplace_back(move(block));
//             }
//         }
//     }
//     return helpers;
// }

// TODO(Prism migration): body wrapping for helpers not needed for signature translation.
// unique_ptr<parser::Node> maybeWrapBodyPrism(unique_ptr<parser::Node> &owner, unique_ptr<parser::Node> body) {
//     if (body == nullptr) {
//         return make_unique<parser::Begin>(owner->loc, parser::NodeVec());
//     } else if (parser::isa_node<parser::Begin>(body.get())) {
//         return body;
//     } else {
//         auto newBody = make_unique<parser::Begin>(body->loc, parser::NodeVec());
//         newBody->stmts.emplace_back(move(body));
//         return newBody;
//     }
// }

// TODO(Prism migration): helpers utilities not needed for signature translation.
// bool containsExtendTHelperPrism(parser::Begin *body) {
//     for (auto &stmt : body->stmts) {
//         auto send = parser::cast_node<parser::Send>(stmt.get());
//         if (send == nullptr) {
//             continue;
//         }

//         if (send->method != core::Names::extend()) {
//             continue;
//         }

//         if (send->receiver != nullptr && !parser::isa_node<parser::Self>(send->receiver.get())) {
//             continue;
//         }

//         if (send->args.size() != 1) {
//             continue;
//         }

//         auto arg = parser::cast_node<parser::Const>(send->args[0].get());
//         if (arg == nullptr) {
//             continue;
//         }

//         if (arg->name != core::Names::Constants::Helpers() || !parser::MK::isT(arg->scope)) {
//             continue;
//         }

//         return true;
//     }

//     return false;
// }

// TODO(Prism migration): helpers utilities not needed for signature translation.
// void maybeInsertExtendTHelpersPrism(unique_ptr<parser::Node> *body) {
//     auto begin = parser::cast_node<parser::Begin>(body->get());
//     ENFORCE(begin != nullptr);

//     if (containsExtendTHelperPrism(begin)) {
//         return;
//     }

//     auto send = parser::MK::Send1(begin->loc, parser::MK::Self(begin->loc), core::Names::extend(), begin->loc,
//                                   parser::MK::T_Helpers(begin->loc));

//     begin->stmts.emplace_back(move(send));
// }

// TODO(Prism migration): helpers utilities not needed for signature translation.
// void insertHelpersPrism(unique_ptr<parser::Node> *body, parser::NodeVec helpers) {
//     auto begin = parser::cast_node<parser::Begin>(body->get());
//     ENFORCE(begin != nullptr);

//     for (auto &helper : helpers) {
//         begin->stmts.emplace_back(move(helper));
//     }
// }

} // namespace

// TODO(Prism migration): type params insertion not required for signature translation.
// void SigsRewriterPrism::insertTypeParams(parser::Node *node, unique_ptr<parser::Node> *body) {
//     ENFORCE(parser::isa_node<parser::Class>(node) || parser::isa_node<parser::Module>(node) ||
//                 parser::isa_node<parser::SClass>(node),
//             "Unimplemented node type: {}", node->nodeName());

//     // NOTE: Prism migration ongoing; insertTypeParams still operates on parser nodes.
//     auto comments = commentsForNode((pm_node_t *)nullptr);
//     if (comments.signatures.empty()) {
//         return;
//     }

//     if (comments.signatures.size() > 1) {
//         if (auto e = ctx.beginIndexerError(comments.signatures[0].commentLoc(),
//                                            core::errors::Rewriter::RBSMultipleGenericSignatures)) {
//             e.setHeader("Generic classes and modules can only have one RBS generic signature");
//             return;
//         }
//     }

//     auto signature = comments.signatures[0];
//     auto typeParamsTranslator = SignatureTranslator(ctx);
//     auto typeParams = typeParamsTranslator.translateTypeParams(signature);

//     if (typeParams.empty()) {
//         return;
//     }

//     auto begin = parser::cast_node<parser::Begin>(body->get());
//     ENFORCE(begin != nullptr);

//     for (auto &typeParam : typeParams) {
//         begin->stmts.emplace_back(move(typeParam));
//     }
// }

CommentsPrism SigsRewriterPrism::commentsForNode(pm_node_t *node) {
    auto comments = CommentsPrism{};
    enum class SignatureState { None, Started, Multiline };
    auto state = SignatureState::None;

    if (commentsByNode != nullptr && node != nullptr) {
        if (auto it = commentsByNode->find(node); it != commentsByNode->end()) {
            auto &nodes = it->second;
            auto declaration_comments = vector<Comment>();

            for (auto &commentNode : nodes) {
                if (absl::StartsWith(commentNode.string, "# @")) {
                    auto comment = Comment{
                        .commentLoc = commentNode.loc,
                        .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 3, commentNode.loc.endPos()},
                        .string = commentNode.string.substr(3),
                    };
                    comments.annotations.emplace_back(move(comment));
                    continue;
                }

                if (absl::StartsWith(commentNode.string, "#:")) {
                    if (state == SignatureState::Multiline) {
                        if (auto e = ctx.beginIndexerError(commentNode.loc,
                                                           core::errors::Rewriter::RBSMultilineMisformatted)) {
                            e.setHeader("Signature start (\"#:\") cannot appear after a multiline signature (\"#|\")");
                            return comments;
                        }
                    }
                    state = SignatureState::Started;
                    auto comment = Comment{
                        .commentLoc = commentNode.loc,
                        .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 2, commentNode.loc.endPos()},
                        .string = commentNode.string.substr(2),
                    };

                    if (declaration_comments.empty()) {
                        declaration_comments.emplace_back(move(comment));
                    } else {
                        comments.signatures.emplace_back(RBSDeclaration{move(declaration_comments)});
                        declaration_comments.clear();
                        declaration_comments.emplace_back(move(comment));
                    }
                    continue;
                }

                if (absl::StartsWith(commentNode.string, "#|")) {
                    if (state == SignatureState::None) {
                        if (auto e = ctx.beginIndexerError(commentNode.loc,
                                                           core::errors::Rewriter::RBSMultilineMisformatted)) {
                            e.setHeader("Multiline signature (\"#|\") must be preceded by a signature start (\"#:\")");
                            return comments;
                        }
                    }
                    state = SignatureState::Multiline;
                    auto comment = Comment{
                        .commentLoc = commentNode.loc,
                        .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 2, commentNode.loc.endPos()},
                        .string = commentNode.string.substr(2),
                    };
                    declaration_comments.emplace_back(move(comment));
                    continue;
                }
            }

            if (!declaration_comments.empty()) {
                auto rbsDeclaration = RBSDeclaration{move(declaration_comments)};
                comments.signatures.emplace_back(move(rbsDeclaration));
            }
        }
    }

    return comments;
}

unique_ptr<vector<pm_node_t *>> SigsRewriterPrism::signaturesForNode(pm_node_t *node) {
    auto comments = commentsForNode(node);

    if (comments.signatures.empty()) {
        return nullptr;
    }

    auto signatures = make_unique<vector<pm_node_t *>>();
    auto signatureTranslator = rbs::SignatureTranslatorPrism(ctx, parser);

    for (auto &declaration : comments.signatures) {
        if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
            auto sig = signatureTranslator.translateMethodSignature(node, declaration, comments.annotations);
            if (sig) {
                signatures->emplace_back(sig);
            }
        } else if (PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
            auto *call = down_cast<pm_call_node_t>(node);
            // TODO: Implement isVisibilitySend and isAttrAccessorSend for Prism nodes
            // This requires checking the method name and context like the original:
            // if (isVisibilitySendPrism(call)) {
            //     auto sig = signatureTranslator.translateMethodSignature(call->arguments->arguments.nodes[0],
            //     declaration, comments.annotations); signatures->emplace_back(move(sig));
            // } else if (isAttrAccessorSendPrism(call)) {
            //     auto sig = signatureTranslator.translateAttrSignature(call, declaration, comments.annotations);
            //     signatures->emplace_back(move(sig));
            // } else {
            //     Exception::raise("Unimplemented call node type");
            // }
            (void)call;        // Suppress unused warning
            (void)declaration; // Suppress unused warning
        } else {
            Exception::raise("Unimplemented node type for signatures: {}", (int)PM_NODE_TYPE(node));
        }
    }

    return signatures;
}

/**
 * Replace the synthetic node with a `T.type_alias` call.
 */
// unique_ptr<parser::Node> SigsRewriterPrism::replaceSyntheticTypeAlias(unique_ptr<parser::Node> node) {
//     auto comments = commentsForNode(node.get());

//     if (comments.signatures.empty()) {
//         // This should never happen
//         Exception::raise("No inline comment found for synthetic type alias");
//     }

//     if (comments.signatures.size() > 1) {
//         // This should never happen
//         Exception::raise("Multiple signatures found for synthetic type alias");
//     }

//     auto aliasDeclaration = comments.signatures[0];
//     auto typeBeginLoc = (uint32_t)aliasDeclaration.string.find("=");

//     auto typeDeclaration = RBSDeclaration{vector<Comment>{Comment{
//         .commentLoc = aliasDeclaration.commentLoc(),
//         .typeLoc = core::LocOffsets{aliasDeclaration.fullTypeLoc().beginPos() + typeBeginLoc + 1,
//                                     aliasDeclaration.fullTypeLoc().endPos()},
//         .string = aliasDeclaration.string.substr(typeBeginLoc + 1),
//     }}};

//     auto type = SignatureTranslator(ctx).translateType(typeDeclaration);

//     if (type == nullptr) {
//         type = parser::MK::TUntyped(node->loc);
//     }

//     return parser::MK::TTypeAlias(type->loc, move(type));
// }

void SigsRewriterPrism::rewriteNodes(pm_node_list_t &nodes) {
    for (size_t i = 0; i < nodes.size; i++) {
        nodes.nodes[i] = rewriteNode(nodes.nodes[i]);
    }
}

pm_node_t *SigsRewriterPrism::rewriteBody(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }
    fmt::print("rewriting body: {}\n", PM_NODE_TYPE(node));

    // Handle statements nodes (class/module bodies with multiple statements)
    if (PM_NODE_TYPE_P(node, PM_STATEMENTS_NODE)) {
        auto *statements = down_cast<pm_statements_node_t>(node);

        // Save old statements
        pm_node_list_t oldStmts = statements->body;
        statements->body = (pm_node_list_t){.size = 0, .capacity = 0, .nodes = nullptr};

        // For each statement, insert signatures before it if it's a signature target
        for (size_t i = 0; i < oldStmts.size; i++) {
            pm_node_t *stmt = oldStmts.nodes[i];

            if (auto target = signaturesTargetForPrism(stmt)) {
                if (auto signatures = signaturesForNode(target)) {
                    // Add all signatures
                    for (auto sig : *signatures) {
                        pm_node_list_append(&statements->body, sig);
                    }
                }
            }

            // Add the rewritten statement
            pm_node_list_append(&statements->body, rewriteNode(stmt));
        }

        // Free the old list structure (not the nodes themselves, as they were moved)
        free(oldStmts.nodes);

        return node;
    }

    // Handle single node that is a signature target
    if (auto target = signaturesTargetForPrism(node)) {
        if (auto signatures = signaturesForNode(target)) {
            // Wrap in a statements node with signatures + node
            return createStatementsWithSignatures(rewriteNode(node), move(signatures));
        }
    }

    return rewriteNode(node);
}

pm_node_t *SigsRewriterPrism::rewriteClass(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    auto comments = commentsForNode(node);
    // TODO: Implement extractHelpers for Prism - extracts @abstract, @interface, @final, etc.
    // auto helpers = extractHelpers(ctx, comments.annotations);

    if (comments.signatures.empty()) {
        // TODO: When helpers are implemented, also check: && helpers.empty()
        return node;
    }

    switch (PM_NODE_TYPE(node)) {
        case PM_CLASS_NODE: {
            auto *klass = down_cast<pm_class_node_t>(node);
            (void)klass; // Suppress unused variable warning
            // TODO: Implement body wrapping and helper insertion for Prism
            // This should follow the same pattern as the original:
            // 1. klass->body = maybeWrapBody(node, move(klass->body));
            // 2. if (!helpers.empty()) {
            //        maybeInsertExtendTHelpers(&klass->body);
            //        insertHelpers(&klass->body, move(helpers));
            //    }
            // 3. insertTypeParams(klass, &klass->body);
            break;
        }
        case PM_MODULE_NODE: {
            auto *module = down_cast<pm_module_node_t>(node);
            (void)module; // Suppress unused variable warning
            // TODO: Same implementation as class case
            break;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = down_cast<pm_singleton_class_node_t>(node);
            (void)sclass; // Suppress unused variable warning
            // TODO: Same implementation as class case
            break;
        }
        default:
            Exception::raise("Unimplemented node type for rewriteClass: {}", (int)PM_NODE_TYPE(node));
    }

    return node;
}

pm_node_t *SigsRewriterPrism::rewriteNode(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    fmt::print("rewriting node: {}\n", PM_NODE_TYPE(node));

    switch (PM_NODE_TYPE(node)) {
        case PM_BLOCK_NODE: {
            auto *block = down_cast<pm_block_node_t>(node);
            block->body = rewriteBody(block->body);
            return node;
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_and_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_or_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_operator_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_MODULE_NODE: {
            auto *mod = down_cast<pm_module_node_t>(node);
            mod->body = rewriteBody(mod->body);
            return node;
        }
        case PM_CLASS_NODE: {
            auto *cls = down_cast<pm_class_node_t>(node);
            cls->body = rewriteBody(cls->body);
            return node;
        }
        case PM_DEF_NODE: {
            auto *def = down_cast<pm_def_node_t>(node);
            def->body = rewriteBody(def->body);
            return node;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = down_cast<pm_singleton_class_node_t>(node);
            sclass->body = rewriteBody(sclass->body);
            return node;
        }
        case PM_FOR_NODE: {
            auto *for_ = down_cast<pm_for_node_t>(node);
            if (for_->statements) {
                for_->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(for_->statements)));
            }
            return node;
        }
        case PM_ARRAY_NODE: {
            auto *array = down_cast<pm_array_node_t>(node);
            rewriteNodes(array->elements);
            return node;
        }
        case PM_HASH_NODE: {
            auto *hash = down_cast<pm_hash_node_t>(node);
            rewriteNodes(hash->elements);
            return node;
        }
        case PM_ASSOC_NODE: {
            auto *pair = down_cast<pm_assoc_node_t>(node);
            pair->key = rewriteNode(pair->key);
            pair->value = rewriteNode(pair->value);
            return node;
        }
        case PM_RESCUE_NODE: {
            auto *rescue = down_cast<pm_rescue_node_t>(node);
            if (rescue->statements) {
                rescue->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(rescue->statements)));
            }
            if (rescue->subsequent) {
                rescue->subsequent = down_cast<pm_rescue_node_t>(rewriteNode(up_cast(rescue->subsequent)));
            }
            return node;
        }
        case PM_ELSE_NODE: {
            auto *else_ = down_cast<pm_else_node_t>(node);
            if (else_->statements) {
                else_->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(else_->statements)));
            }
            return node;
        }
        case PM_ENSURE_NODE: {
            auto *ensure = down_cast<pm_ensure_node_t>(node);
            if (ensure->statements) {
                ensure->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(ensure->statements)));
            }
            return node;
        }
        case PM_IF_NODE: {
            auto *if_ = down_cast<pm_if_node_t>(node);
            if (if_->statements) {
                if_->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(if_->statements)));
            }
            if (if_->subsequent) {
                if_->subsequent = rewriteBody(if_->subsequent);
            }
            return node;
        }
        case PM_MULTI_WRITE_NODE: {
            auto *masgn = down_cast<pm_multi_write_node_t>(node);
            masgn->value = rewriteNode(masgn->value);
            return node;
        }
        case PM_CASE_NODE: {
            auto *case_ = down_cast<pm_case_node_t>(node);
            rewriteNodes(case_->conditions);
            if (case_->else_clause) {
                case_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(case_->else_clause)));
            }
            return node;
        }
        case PM_CASE_MATCH_NODE: {
            auto *case_ = down_cast<pm_case_match_node_t>(node);
            rewriteNodes(case_->conditions);
            if (case_->else_clause) {
                case_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(case_->else_clause)));
            }
            return node;
        }
        case PM_WHEN_NODE: {
            auto *when = down_cast<pm_when_node_t>(node);
            if (when->statements) {
                when->statements = down_cast<pm_statements_node>(rewriteBody(up_cast(when->statements)));
            }
            return node;
        }
        case PM_CALL_NODE: {
            auto *call = down_cast<pm_call_node_t>(node);
            if (call->arguments) {
                rewriteNodes(call->arguments->arguments);
            }
            return node;
        }
        case PM_SUPER_NODE: {
            auto *sup = down_cast<pm_super_node_t>(node);
            if (sup->arguments) {
                rewriteNodes(sup->arguments->arguments);
            }
            return node;
        }
        case PM_RETURN_NODE: {
            auto *ret = down_cast<pm_return_node_t>(node);
            if (ret->arguments) {
                rewriteNodes(ret->arguments->arguments);
            }
            return node;
        }
        case PM_NEXT_NODE: {
            auto *n = down_cast<pm_next_node_t>(node);
            if (n->arguments) {
                rewriteNodes(n->arguments->arguments);
            }
            return node;
        }
        case PM_BREAK_NODE: {
            auto *b = down_cast<pm_break_node_t>(node);
            if (b->arguments) {
                rewriteNodes(b->arguments->arguments);
            }
            return node;
        }
        case PM_SPLAT_NODE: {
            auto *s = down_cast<pm_splat_node_t>(node);
            s->expression = rewriteNode(s->expression);
            return node;
        }
        case PM_PROGRAM_NODE: {
            auto *program = down_cast<pm_program_node_t>(node);
            rewriteBody(up_cast(program->statements));
            return node;
        }
        case PM_STATEMENTS_NODE: {
            auto *statements = down_cast<pm_statements_node_t>(node);
            rewriteNodes(statements->body);
            return node;
        }
        default:
            return node;
    }
}

pm_node_t *SigsRewriterPrism::run(pm_node_t *node) {
    return rewriteBody(node);
}

// Helper method to create statements nodes with signatures
pm_node_t *SigsRewriterPrism::createStatementsWithSignatures(pm_node_t *originalNode,
                                                             std::unique_ptr<std::vector<pm_node_t *>> signatures) {
    if (!signatures || signatures->empty()) {
        return originalNode; // No signatures, return original node
    }

    // Create a statements node that wraps the signature + original method
    pm_parser_t *p = parser.getInternalParser();

    // Create the statements node
    pm_statements_node_t *stmts = (pm_statements_node_t *)calloc(1, sizeof(pm_statements_node_t));
    if (!stmts)
        return originalNode;

    *stmts = (pm_statements_node_t){
        .base = {.type = PM_STATEMENTS_NODE,
                 .flags = 0,
                 .node_id = ++p->node_id,
                 .location = {.start = originalNode->location.start, .end = originalNode->location.end}},
        .body = {.size = 0, .capacity = 0, .nodes = nullptr}};

    // Add all the Prism signature nodes
    for (auto sigCall : *signatures) {
        if (sigCall) {
            addNodeToStatements(stmts, sigCall);
        }
    }

    // Add the original method
    addNodeToStatements(stmts, originalNode);

    return up_cast(stmts);
}

// Helper to add a node to a statements node
bool SigsRewriterPrism::addNodeToStatements(pm_statements_node_t *stmts, pm_node_t *node) {
    if (!stmts || !node)
        return false;

    // Grow the node list if needed
    if (stmts->body.size >= stmts->body.capacity) {
        size_t new_capacity = stmts->body.capacity == 0 ? 4 : stmts->body.capacity * 2;
        pm_node_t **new_nodes = (pm_node_t **)realloc(stmts->body.nodes, sizeof(pm_node_t *) * new_capacity);
        if (!new_nodes)
            return false;

        stmts->body.nodes = new_nodes;
        stmts->body.capacity = new_capacity;
    }

    // Add the node
    stmts->body.nodes[stmts->body.size++] = node;

    // Update the statements location to encompass the new node
    if (stmts->body.size == 1) {
        // First node - set the statements bounds
        stmts->base.location.start = node->location.start;
        stmts->base.location.end = node->location.end;
    } else {
        // Expand bounds to include new node
        if (node->location.start < stmts->base.location.start) {
            stmts->base.location.start = node->location.start;
        }
        if (node->location.end > stmts->base.location.end) {
            stmts->base.location.end = node->location.end;
        }
    }

    return true;
}

} // namespace sorbet::rbs
