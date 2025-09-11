#include "rbs/prism/SigsRewriterPrism.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"
#include "rbs/TypeParamsToParserNodes.h"

using namespace std;

namespace sorbet::rbs {

namespace {

// Prism version of signaturesTarget
pm_node_t *signaturesTargetForPrism(pm_node_t *node) {
    if (node == nullptr) {
        return nullptr;
    }

    switch (node->type) {
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
unique_ptr<parser::Node> extractHelperArgumentPrism(core::MutableContext ctx, Comment annotation, int offset) {
    while (annotation.string[offset] == ' ') {
        offset++;
    }

    Comment comment = {
        core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
        core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
        annotation.string.substr(offset),
    };

    return rbs::SignatureTranslator(ctx).translateType(RBSDeclaration{vector<Comment>{comment}});
}

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

unique_ptr<parser::NodeVec> SigsRewriterPrism::signaturesForNode(pm_node_t *node) {
    auto comments = commentsForNode(node);

    if (comments.signatures.empty()) {
        return nullptr;
    }

    auto signatures = make_unique<parser::NodeVec>();
    auto signatureTranslator = rbs::SignatureTranslator(ctx);

    for (auto &declaration : comments.signatures) {
        switch (node->type) {
            case PM_DEF_NODE: {
                // Both instance and singleton methods use PM_DEF_NODE in Prism
                // TODO: Pass the correct parser::Node* to translateMethodSignature
                // For now, this is stubbed until we have proper Prism->Parser node conversion
                // auto sig = signatureTranslator.translateMethodSignature(prismToParserNode(node), declaration, comments.annotations);
                // signatures->emplace_back(move(sig));
                (void)declaration; // Suppress unused warning
                break;
            }
            case PM_CALL_NODE: {
                auto *call = (pm_call_node_t *)node;
                // TODO: Implement isVisibilitySend and isAttrAccessorSend for Prism nodes
                // This requires checking the method name and context like the original:
                // if (isVisibilitySendPrism(call)) {
                //     auto sig = signatureTranslator.translateMethodSignature(call->arguments->arguments.nodes[0], declaration, comments.annotations);
                //     signatures->emplace_back(move(sig));
                // } else if (isAttrAccessorSendPrism(call)) {
                //     auto sig = signatureTranslator.translateAttrSignature(call, declaration, comments.annotations);
                //     signatures->emplace_back(move(sig));
                // } else {
                //     Exception::raise("Unimplemented call node type");
                // }
                (void)call; // Suppress unused warning
                (void)declaration; // Suppress unused warning
                break;
            }
            default:
                Exception::raise("Unimplemented node type for signatures: {}", (int)node->type);
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

pm_node_t *SigsRewriterPrism::rewriteBegin(pm_node_t *node) {
    if (node->type != PM_BEGIN_NODE) {
        return node;
    }

    auto *begin = (pm_begin_node_t *)node;
    if (begin->statements == nullptr) {
        return node;
    }

    // TODO: Implement signature insertion logic for Prism
    // This should follow the same pattern as the original rewriteBegin:
    //
    // 1. Save old statements
    // 2. Clear the statements list
    // 3. For each statement:
    //    - Check if it's a signature target (method/attr/visibility)
    //    - If yes, insert signatures before the statement
    //    - Add the rewritten statement
    //
    // Original logic:
    // auto oldStmts = move(begin->stmts);
    // begin->stmts = parser::NodeVec();
    // for (auto &stmt : oldStmts) {
    //     if (auto target = signaturesTarget(stmt.get())) {
    //         if (auto signatures = signaturesForNode(target)) {
    //             for (auto &declaration : *signatures) {
    //                 begin->stmts.emplace_back(move(declaration));
    //             }
    //         }
    //     }
    //     begin->stmts.emplace_back(rewriteNode(move(stmt)));
    // }

    // For now, just rewrite the existing statements
    for (size_t i = 0; i < begin->statements->body.size; i++) {
        begin->statements->body.nodes[i] = rewriteNode(begin->statements->body.nodes[i]);
    }

    return node;
}

pm_node_t *SigsRewriterPrism::rewriteBody(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    if (node->type == PM_BEGIN_NODE) {
        return rewriteBegin(node);
    }

    if (auto target = signaturesTargetForPrism(node)) {
        auto comments = commentsForNode(target);
        if (!comments.signatures.empty()) {
            // TODO: For signature insertion, we need to create a new PM_BEGIN_NODE
            // and insert signature nodes before the target node.
            // This requires:
            // 1. Creating signature parser nodes from RBS comments
            // 2. Creating a new PM_BEGIN_NODE structure  
            // 3. Adding signatures + target node to the begin block
            //
            // For now, just return the node as-is until we implement
            // the signature-to-parser-node translation and begin node creation
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

    switch (node->type) {
        case PM_CLASS_NODE: {
            auto *klass = (pm_class_node_t *)node;
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
            auto *module = (pm_module_node_t *)node;
            (void)module; // Suppress unused variable warning
            // TODO: Same implementation as class case
            break;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = (pm_singleton_class_node_t *)node;
            (void)sclass; // Suppress unused variable warning
            // TODO: Same implementation as class case
            break;
        }
        default:
            Exception::raise("Unimplemented node type for rewriteClass: {}", (int)node->type);
    }

    return node;
}

pm_node_t *SigsRewriterPrism::rewriteNode(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }
    switch (node->type) {
        case PM_BLOCK_NODE: {
            auto *block = (pm_block_node_t *)node;
            block->body = rewriteBody(block->body);
            return node;
        }
        case PM_BEGIN_NODE: {
            return rewriteBegin(node);
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: {
            auto *n = (pm_local_variable_write_node_t *)node;
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: {
            auto *n = (pm_local_variable_and_write_node_t *)node;
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: {
            auto *n = (pm_local_variable_or_write_node_t *)node;
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *n = (pm_local_variable_operator_write_node_t *)node;
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_MODULE_NODE: {
            auto *mod = (pm_module_node_t *)node;
            mod->body = rewriteBody(mod->body);
            return node;
        }
        case PM_CLASS_NODE: {
            auto *cls = (pm_class_node_t *)node;
            cls->body = rewriteBody(cls->body);
            return node;
        }
        case PM_DEF_NODE: {
            auto *def = (pm_def_node_t *)node;
            def->body = rewriteBody(def->body);
            return node;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = (pm_singleton_class_node_t *)node;
            sclass->body = rewriteBody(sclass->body);
            return node;
        }
        case PM_FOR_NODE: {
            auto *for_ = (pm_for_node_t *)node;
            if (for_->statements) {
                for_->statements = (pm_statements_node *)rewriteBody((pm_node_t *)for_->statements);
            }
            return node;
        }
        case PM_ARRAY_NODE: {
            auto *array = (pm_array_node_t *)node;
            rewriteNodes(array->elements);
            return node;
        }
        case PM_HASH_NODE: {
            auto *hash = (pm_hash_node_t *)node;
            rewriteNodes(hash->elements);
            return node;
        }
        case PM_ASSOC_NODE: {
            auto *pair = (pm_assoc_node_t *)node;
            pair->key = rewriteNode(pair->key);
            pair->value = rewriteNode(pair->value);
            return node;
        }
        case PM_RESCUE_NODE: {
            auto *rescue = (pm_rescue_node_t *)node;
            if (rescue->statements) {
                rescue->statements = (pm_statements_node *)rewriteBody((pm_node_t *)rescue->statements);
            }
            if (rescue->subsequent) {
                rescue->subsequent = (pm_rescue_node_t *)rewriteNode((pm_node_t *)rescue->subsequent);
            }
            return node;
        }
        case PM_ELSE_NODE: {
            auto *else_ = (pm_else_node_t *)node;
            if (else_->statements) {
                else_->statements = (pm_statements_node *)rewriteBody((pm_node_t *)else_->statements);
            }
            return node;
        }
        case PM_ENSURE_NODE: {
            auto *ensure = (pm_ensure_node_t *)node;
            if (ensure->statements) {
                ensure->statements = (pm_statements_node *)rewriteBody((pm_node_t *)ensure->statements);
            }
            return node;
        }
        case PM_IF_NODE: {
            auto *if_ = (pm_if_node_t *)node;
            if (if_->statements) {
                if_->statements = (pm_statements_node *)rewriteBody((pm_node_t *)if_->statements);
            }
            if (if_->subsequent) {
                if_->subsequent = rewriteBody(if_->subsequent);
            }
            return node;
        }
        case PM_MULTI_WRITE_NODE: {
            auto *masgn = (pm_multi_write_node_t *)node;
            masgn->value = rewriteNode(masgn->value);
            return node;
        }
        case PM_CASE_NODE: {
            auto *case_ = (pm_case_node_t *)node;
            rewriteNodes(case_->conditions);
            if (case_->else_clause) {
                case_->else_clause = (pm_else_node_t *)rewriteBody((pm_node_t *)case_->else_clause);
            }
            return node;
        }
        case PM_CASE_MATCH_NODE: {
            auto *case_ = (pm_case_match_node_t *)node;
            rewriteNodes(case_->conditions);
            if (case_->else_clause) {
                case_->else_clause = (pm_else_node_t *)rewriteBody((pm_node_t *)case_->else_clause);
            }
            return node;
        }
        case PM_WHEN_NODE: {
            auto *when = (pm_when_node_t *)node;
            if (when->statements) {
                when->statements = (pm_statements_node *)rewriteBody((pm_node_t *)when->statements);
            }
            return node;
        }
        case PM_CALL_NODE: {
            auto *call = (pm_call_node_t *)node;
            if (call->arguments) {
                rewriteNodes(call->arguments->arguments);
            }
            return node;
        }
        case PM_SUPER_NODE: {
            auto *sup = (pm_super_node_t *)node;
            if (sup->arguments) {
                rewriteNodes(sup->arguments->arguments);
            }
            return node;
        }
        case PM_RETURN_NODE: {
            auto *ret = (pm_return_node_t *)node;
            if (ret->arguments) {
                rewriteNodes(ret->arguments->arguments);
            }
            return node;
        }
        case PM_NEXT_NODE: {
            auto *n = (pm_next_node_t *)node;
            if (n->arguments) {
                rewriteNodes(n->arguments->arguments);
            }
            return node;
        }
        case PM_BREAK_NODE: {
            auto *b = (pm_break_node_t *)node;
            if (b->arguments) {
                rewriteNodes(b->arguments->arguments);
            }
            return node;
        }
        case PM_SPLAT_NODE: {
            auto *s = (pm_splat_node_t *)node;
            s->expression = rewriteNode(s->expression);
            return node;
        }
        default:
            return node;
    }
}

pm_node_t *SigsRewriterPrism::run(pm_node_t *node) {
    return rewriteBody(node);
}

} // namespace sorbet::rbs
