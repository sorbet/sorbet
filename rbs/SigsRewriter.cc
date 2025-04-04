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

Comments signaturesForLoc(core::MutableContext ctx, core::LocOffsets loc) {
    auto source = ctx.file.data(ctx).source();

    vector<Comment> annotations;
    vector<Comment> signatures;

    uint32_t beginIndex = loc.beginPos();

    // Everything in the file before the method definition
    string_view preDefinition = source.substr(0, source.rfind('\n', beginIndex));

    // Get all the lines before it
    vector<string_view> all_lines = absl::StrSplit(preDefinition, '\n');

    // We compute the current position in the source so we know the location of each comment
    uint32_t index = beginIndex;

    // NOTE: This is accidentally quadratic.
    // Instead of looping over all the lines between here and the start of the file, we should
    // instead track something like the locs of all the expressions in the ClassDef::rhs, and
    // only scan over the space between the ClassDef::rhs top level items

    // Iterate from the last line, to the first line
    for (auto it = all_lines.rbegin(); it != all_lines.rend(); it++) {
        index -= it->size();
        index -= 1;

        string_view line = absl::StripAsciiWhitespace(*it);

        // Short circuit when line is empty
        if (line.empty()) {
            break;
        }

        // Handle single-line sig block
        else if (absl::StartsWith(line, "sig")) {
            // Do nothing for a one-line sig block
            // TODO: Handle single-line sig blocks
        }

        // Handle multi-line sig block
        else if (absl::StartsWith(line, "end")) {
            // ASSUMPTION: We either hit the start of file, a `sig do`/`sig(:final) do` or an `end`
            // TODO: Handle multi-line sig blocks
            it++;
            while (
                // SOF
                it != all_lines.rend()
                // Start of sig block
                && !(absl::StartsWith(absl::StripAsciiWhitespace(*it), "sig do") ||
                     absl::StartsWith(absl::StripAsciiWhitespace(*it), "sig(:final) do"))
                // Invalid end keyword
                && !absl::StartsWith(absl::StripAsciiWhitespace(*it), "end")) {
                it++;
            };

            // We have either
            // 1) Reached the start of the file
            // 2) Found a `sig do`
            // 3) Found an invalid end keyword
            if (it == all_lines.rend() || absl::StartsWith(absl::StripAsciiWhitespace(*it), "end")) {
                break;
            }

            // Reached a sig block.
            line = absl::StripAsciiWhitespace(*it);
            ENFORCE(absl::StartsWith(line, "sig do") || absl::StartsWith(line, "sig(:final) do"));

            // Stop looking if this is a single-line block e.g `sig do; <block>; end`
            if ((absl::StartsWith(line, "sig do;") || absl::StartsWith(line, "sig(:final) do;")) &&
                absl::EndsWith(line, "end")) {
                break;
            }

            // Else, this is a valid sig block. Move on to any possible documentation.
        }

        // Handle a RBS sig annotation `#: SomeRBS`
        else if (absl::StartsWith(line, "#:")) {
            // Account for whitespace before the annotation e.g
            // #: abc -> "abc"
            // #:abc -> "abc"
            int lineSize = line.size();
            auto rbsSignature = Comment{
                core::LocOffsets{index, index + lineSize},
                core::LocOffsets{index + 2, index + lineSize},
                line.substr(2),
            };
            signatures.emplace_back(rbsSignature);
        }

        // Handle RDoc annotations `# @abstract`
        else if (absl::StartsWith(line, "# @")) {
            int lineSize = line.size();
            auto annotation = Comment{
                core::LocOffsets{index, index + lineSize},
                core::LocOffsets{index + 3, index + lineSize},
                line.substr(3),
            };
            annotations.emplace_back(annotation);
        }

        // Ignore other comments
        else if (absl::StartsWith(line, "#")) {
            continue;
        }

        // No other cases applied to this line, so stop looking.
        else {
            break;
        }
    }

    reverse(annotations.begin(), annotations.end());
    reverse(signatures.begin(), signatures.end());

    return Comments{annotations, signatures};
}

unique_ptr<parser::NodeVec> signaturesForNode(core::MutableContext ctx, parser::Node *node) {
    auto comments = signaturesForLoc(ctx, node->loc);

    if (comments.signatures.empty()) {
        return nullptr;
    }

    auto signatures = make_unique<parser::NodeVec>();
    auto signatureTranslator = rbs::SignatureTranslator(ctx);

    for (auto &signature : comments.signatures) {
        if (parser::isa_node<parser::DefMethod>(node) || parser::isa_node<parser::DefS>(node)) {
            auto sig = signatureTranslator.translateMethodSignature(node, signature, comments.annotations);

            signatures->emplace_back(move(sig));
        } else if (auto send = parser::cast_node<parser::Send>(node)) {
            auto sig = signatureTranslator.translateAttrSignature(send, signature, comments.annotations);
            signatures->emplace_back(move(sig));
        } else {
            Exception::raise("Unimplemented node type: {}", node->nodeName());
        }
    }

    return signatures;
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

    return rbs::SignatureTranslator(ctx).translateType(
        core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
        annotation.string.substr(offset));
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
        } else if (absl::StartsWith(annotation.string, "mixes_in_class_methods:")) {
            if (auto type = extractHelperArgument(ctx, annotation, 23)) {
                auto send = parser::MK::Send1(annotation.typeLoc, parser::MK::Self(annotation.typeLoc),
                                              core::Names::mixesInClassMethods(), annotation.typeLoc, move(type));
                helpers.emplace_back(move(send));
            }
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
                for (auto &signature : *signatures) {
                    begin->stmts.emplace_back(move(signature));
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
            for (auto &signature : *signatures) {
                begin->stmts.emplace_back(move(signature));
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

    auto comments = signaturesForLoc(ctx, node->loc);
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
