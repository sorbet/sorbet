#include "rbs/SigsRewriter.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rbs {

namespace {

bool isVisibilitySend(const parser::Send *send) {
    return send->receiver == nullptr && send->args.size() == 1 &&
           (parser::isa_node<parser::DefMethod>(send->args[0].get()) ||
            parser::isa_node<parser::DefS>(send->args[0].get())) &&
           (send->method == core::Names::private_() || send->method == core::Names::protected_() ||
            send->method == core::Names::public_() || send->method == core::Names::privateClassMethod() ||
            send->method == core::Names::publicClassMethod() || send->method == core::Names::packagePrivate() ||
            send->method == core::Names::packagePrivateClassMethod());
}

bool isAttrAccessorSend(const parser::Send *send) {
    return (send->receiver == nullptr || parser::isa_node<parser::Self>(send->receiver.get())) &&
           (send->method == core::Names::attrReader() || send->method == core::Names::attrWriter() ||
            send->method == core::Names::attrAccessor());
}

parser::Node *signaturesTarget(parser::Node *node) {
    if (parser::isa_node<parser::DefMethod>(node) || parser::cast_node<parser::DefS>(node)) {
        return node;
    }

    if (auto send = parser::cast_node<parser::Send>(node)) {
        if (isVisibilitySend(send)) {
            return send->args[0].get();
        } else if (isAttrAccessorSend(send)) {
            return node;
        }
    }

    return nullptr;
}

/**
 * Extracts and parses the argument from the annotation string.
 *
 * Considering an annotation like `@mixes_in_class_methods: ClassMethods`,
 * this function will extract and parse `ClassMethods` as a type then return the corresponding parser::Node.
 *
 * We do not error if the node is not a constant, we just insert it as is and let the pipeline error down the line.
 */
unique_ptr<parser::Node> extractHelperArgument(core::MutableContext ctx, Comment annotation, int offset) {
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

/**
 * Extracts and parses the helpers from the annotations.
 *
 * Returns a `parser::NodeVec`, containing the `parser::Node` corresponding to each annotation.
 *
 * For example, given the annotations the following annotations:
 *
 *     # @abstract,
 *     # @interface
 *
 * This function will return two `parser::Send` nodes:
 *
 *
 * 1. `self.abstract!()`
 * 2. `self.interface!()`
 *
 * It doesn't insert them into the body of the class/module/etc.
 */
parser::NodeVec extractHelpers(core::MutableContext ctx, vector<Comment> annotations) {
    parser::NodeVec helpers;

    for (auto &annotation : annotations) {
        if (annotation.string == "abstract") {
            auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                          core::Names::declareAbstract(), annotation.typeLoc);
            helpers.emplace_back(move(send));
        } else if (annotation.string == "interface") {
            auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                          core::Names::declareInterface(), annotation.typeLoc);
            helpers.emplace_back(move(send));
        } else if (annotation.string == "final") {
            auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                          core::Names::declareFinal(), annotation.typeLoc);
            helpers.emplace_back(move(send));
        } else if (annotation.string == "sealed") {
            auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                          core::Names::declareSealed(), annotation.typeLoc);
            helpers.emplace_back(move(send));
        } else if (absl::StartsWith(annotation.string, "requires_ancestor:")) {
            if (auto type = extractHelperArgument(ctx, annotation, 18)) {
                auto body = make_unique<parser::Begin>(annotation.typeLoc, parser::NodeVec());
                body->stmts.emplace_back(move(type));
                auto send = parser::MK::Send0(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                              core::Names::requiresAncestor(), annotation.typeLoc);
                auto block = make_unique<parser::Block>(annotation.typeLoc, move(send), nullptr, move(body));
                helpers.emplace_back(move(block));
            }
        }
    }

    return helpers;
}

/**
 * Wraps the body in a `parser::Begin` if it isn't already.
 *
 * This is useful for cases where we want to insert helpers into the body of a class/module/etc.
 */
unique_ptr<parser::Node> maybeWrapBody(unique_ptr<parser::Node> &owner, unique_ptr<parser::Node> body) {
    if (body == nullptr) {
        return make_unique<parser::Begin>(owner->loc, parser::NodeVec());
    } else if (parser::isa_node<parser::Begin>(body.get())) {
        return body;
    } else {
        auto newBody = make_unique<parser::Begin>(body->loc, parser::NodeVec());
        newBody->stmts.emplace_back(move(body));
        return newBody;
    }
}

/**
 * Returns true if the body contains an `extend T::Helpers` call already.
 */
bool containsExtendTHelper(parser::Begin *body) {
    for (auto &stmt : body->stmts) {
        auto send = parser::cast_node<parser::Send>(stmt.get());
        if (send == nullptr) {
            continue;
        }

        if (send->method != core::Names::extend()) {
            continue;
        }

        if (send->receiver != nullptr && !parser::isa_node<parser::Self>(send->receiver.get())) {
            continue;
        }

        if (send->args.size() != 1) {
            continue;
        }

        auto arg = parser::cast_node<parser::Const>(send->args[0].get());
        if (arg == nullptr) {
            continue;
        }

        if (arg->name != core::Names::Constants::Helpers() || !parser::MK::isT(arg->scope)) {
            continue;
        }

        return true;
    }

    return false;
}

/**
 * Inserts an `extend T::Helpers` call into the body if it doesn't already exist.
 */
void maybeInsertExtendTHelpers(unique_ptr<parser::Node> *body) {
    auto begin = parser::cast_node<parser::Begin>(body->get());
    ENFORCE(begin != nullptr);

    if (containsExtendTHelper(begin)) {
        return;
    }

    auto send = parser::MK::Send1(begin->loc, parser::MK::Self(begin->loc), core::Names::extend(), begin->loc,
                                  parser::MK::T_Helpers(begin->loc));

    begin->stmts.emplace_back(move(send));
}

/**
 * Inserts the helpers into the body.
 */
void insertHelpers(unique_ptr<parser::Node> *body, parser::NodeVec helpers) {
    auto begin = parser::cast_node<parser::Begin>(body->get());
    ENFORCE(begin != nullptr);

    for (auto &helper : helpers) {
        begin->stmts.emplace_back(move(helper));
    }
}

} // namespace

Comments SigsRewriter::commentsForNode(core::MutableContext ctx, parser::Node *node) {
    auto comments = Comments{};
    enum class SignatureState { None, Started, Multiline };
    auto state = SignatureState::None;

    if (auto commentsNodesEntry = commentsByNode.find(node); commentsNodesEntry != commentsByNode.end()) {
        auto commentsNodes = commentsNodesEntry->second;
        auto declaration_comments = std::vector<Comment>();

        for (auto &commentNode : commentsNodes) {
            // If the comment starts with `# @`, it's an annotation
            if (absl::StartsWith(commentNode.string, "# @")) {
                auto comment = Comment{
                    .commentLoc = commentNode.loc,
                    .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 3, commentNode.loc.endPos()},
                    .string = commentNode.string.substr(3),
                };

                comments.annotations.emplace_back(move(comment));
                continue;
            }

            // If the comment starts with `#:`, it's a signature
            if (absl::StartsWith(commentNode.string, "#:")) {
                if (state == SignatureState::Multiline) {
                    if (auto e = ctx.beginError(commentNode.loc, core::errors::Rewriter::RBSMultilineMisformatted)) {
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
                    // We already have a declaration comment, create a separate RBSDeclaration for better errors
                    // down the line
                    comments.signatures.emplace_back(
                        RBSDeclaration{move(declaration_comments)}); // Save current declaration
                    declaration_comments.clear();
                    declaration_comments.emplace_back(move(comment));
                }
                continue;
            }

            // If the comment starts with `#|`, it's a multiline signature
            if (absl::StartsWith(commentNode.string, "#|")) {
                if (state == SignatureState::None) {
                    if (auto e = ctx.beginError(commentNode.loc, core::errors::Rewriter::RBSMultilineMisformatted)) {
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

    return comments;
}

unique_ptr<parser::NodeVec> SigsRewriter::signaturesForNode(core::MutableContext ctx, parser::Node *node) {
    auto comments = commentsForNode(ctx, node);

    if (comments.signatures.empty()) {
        return nullptr;
    }

    auto signatures = make_unique<parser::NodeVec>();
    auto signatureTranslator = rbs::SignatureTranslator(ctx);

    for (auto &declaration : comments.signatures) {
        if (parser::isa_node<parser::DefMethod>(node) || parser::isa_node<parser::DefS>(node)) {
            auto sig = signatureTranslator.translateMethodSignature(node, declaration, comments.annotations);

            signatures->emplace_back(move(sig));
        } else if (auto send = parser::cast_node<parser::Send>(node)) {
            auto sig = signatureTranslator.translateAttrSignature(send, declaration, comments.annotations);

            signatures->emplace_back(move(sig));
        } else {
            Exception::raise("Unimplemented node type: {}", node->nodeName());
        }
    }

    return signatures;
}

parser::NodeVec SigsRewriter::rewriteNodes(parser::NodeVec nodes) {
    parser::NodeVec result;

    for (auto &node : nodes) {
        result.emplace_back(rewriteBody(move(node)));
    }

    return result;
}

unique_ptr<parser::Node> SigsRewriter::rewriteBegin(unique_ptr<parser::Node> node) {
    auto begin = parser::cast_node<parser::Begin>(node.get());
    ENFORCE(begin != nullptr);

    auto oldStmts = move(begin->stmts);
    begin->stmts = parser::NodeVec();

    for (auto &stmt : oldStmts) {
        if (auto target = signaturesTarget(stmt.get())) {
            if (auto signatures = signaturesForNode(ctx, target)) {
                for (auto &declaration : *signatures) {
                    begin->stmts.emplace_back(move(declaration));
                }
            }
        }

        begin->stmts.emplace_back(rewriteNode(move(stmt)));
    }

    return node;
}

unique_ptr<parser::Node> SigsRewriter::rewriteBody(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    if (parser::isa_node<parser::Begin>(node.get())) {
        return rewriteBegin(move(node));
    }

    if (auto target = signaturesTarget(node.get())) {
        if (auto signatures = signaturesForNode(ctx, target)) {
            auto begin = make_unique<parser::Begin>(node->loc, parser::NodeVec());
            for (auto &declaration : *signatures) {
                begin->stmts.emplace_back(move(declaration));
            }
            begin->stmts.emplace_back(move(node));
            return move(begin);
        }
    }

    return rewriteNode(move(node));
}

unique_ptr<parser::Node> SigsRewriter::rewriteClass(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    auto comments = commentsForNode(ctx, node.get());
    if (comments.annotations.empty()) {
        return node;
    }

    auto helpers = extractHelpers(ctx, comments.annotations);
    if (helpers.empty()) {
        return node;
    }

    typecase(
        node.get(),
        [&](parser::Class *klass) {
            klass->body = maybeWrapBody(node, move(klass->body));
            maybeInsertExtendTHelpers(&klass->body);
            insertHelpers(&klass->body, move(helpers));
        },
        [&](parser::Module *module) {
            module->body = maybeWrapBody(node, move(module->body));
            maybeInsertExtendTHelpers(&module->body);
            insertHelpers(&module->body, move(helpers));
        },
        [&](parser::SClass *sclass) {
            sclass->body = maybeWrapBody(node, move(sclass->body));
            maybeInsertExtendTHelpers(&sclass->body);
            insertHelpers(&sclass->body, move(helpers));
        },
        [&](parser::Node *other) { Exception::raise("Unimplemented node type: {}", other->nodeName()); });

    return node;
}

unique_ptr<parser::Node> SigsRewriter::rewriteNode(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    unique_ptr<parser::Node> result;

    typecase(
        node.get(),
        // Using the same order as Desugar.cc
        [&](parser::Block *block) {
            block->body = rewriteBody(move(block->body));
            result = move(node);
        },
        [&](parser::Begin *begin) {
            node = rewriteBegin(move(node));
            result = move(node);
        },
        [&](parser::Assign *assign) {
            assign->rhs = rewriteNode(move(assign->rhs));
            result = move(node);
        },
        [&](parser::AndAsgn *andAsgn) {
            andAsgn->right = rewriteNode(move(andAsgn->right));
            result = move(node);
        },
        [&](parser::OrAsgn *orAsgn) {
            orAsgn->right = rewriteNode(move(orAsgn->right));
            result = move(node);
        },
        [&](parser::OpAsgn *opAsgn) {
            opAsgn->right = rewriteNode(move(opAsgn->right));
            result = move(node);
        },
        [&](parser::Kwbegin *kwbegin) {
            kwbegin->stmts = rewriteNodes(move(kwbegin->stmts));
            result = move(node);
        },
        [&](parser::Module *module) {
            module->body = rewriteBody(move(module->body));
            result = rewriteClass(move(node));
        },
        [&](parser::Class *klass) {
            klass->body = rewriteBody(move(klass->body));
            result = rewriteClass(move(node));
        },
        [&](parser::DefMethod *def) {
            def->body = rewriteBody(move(def->body));
            result = move(node);
        },
        [&](parser::DefS *def) {
            def->body = rewriteBody(move(def->body));
            result = move(node);
        },
        [&](parser::SClass *sclass) {
            sclass->body = rewriteBody(move(sclass->body));
            result = rewriteClass(move(node));
        },
        [&](parser::For *for_) {
            for_->body = rewriteBody(move(for_->body));
            result = move(node);
        },
        [&](parser::Array *array) {
            array->elts = rewriteNodes(move(array->elts));
            result = move(node);
        },
        [&](parser::Rescue *rescue) {
            rescue->body = rewriteBody(move(rescue->body));
            rescue->rescue = rewriteNodes(move(rescue->rescue));
            rescue->else_ = rewriteBody(move(rescue->else_));
            result = move(node);
        },
        [&](parser::Resbody *resbody) {
            resbody->body = rewriteBody(move(resbody->body));
            result = move(node);
        },
        [&](parser::Ensure *ensure) {
            ensure->body = rewriteBody(move(ensure->body));
            ensure->ensure = rewriteBody(move(ensure->ensure));
            result = move(node);
        },
        [&](parser::If *if_) {
            if_->then_ = rewriteBody(move(if_->then_));
            if_->else_ = rewriteBody(move(if_->else_));
            result = move(node);
        },
        [&](parser::Masgn *masgn) {
            masgn->rhs = rewriteNode(move(masgn->rhs));
            result = move(node);
        },
        [&](parser::Case *case_) {
            case_->whens = rewriteNodes(move(case_->whens));
            case_->else_ = rewriteBody(move(case_->else_));
            result = move(node);
        },
        [&](parser::When *when) {
            when->body = rewriteBody(move(when->body));
            result = move(node);
        },
        [&](parser::Node *other) { result = move(node); });

    return result;
}

unique_ptr<parser::Node> SigsRewriter::run(unique_ptr<parser::Node> node) {
    return rewriteBody(move(node));
}

} // namespace sorbet::rbs
