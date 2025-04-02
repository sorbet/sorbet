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

const string_view RBS_PREFIX = "#:";
const string_view ANNOTATION_PREFIX = "# @";

pair<string_view, uint32_t> stripAsciiWhitespaceWithCount(string_view str) {
    uint32_t prefixWhitespaceCount = 0;
    while (prefixWhitespaceCount < str.size() && absl::ascii_isspace(str[prefixWhitespaceCount])) {
        prefixWhitespaceCount++;
    }
    return {str.substr(prefixWhitespaceCount), prefixWhitespaceCount};
}

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

unique_ptr<parser::NodeVec> signaturesForNode(core::MutableContext ctx, parser::Node *node,
                                              UnorderedMap<uint32_t, SigsRewriter::Comments> &methodSignatures) {
    auto lineStart = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;

    if (auto it = methodSignatures.find(lineStart); it != methodSignatures.end()) {
        auto &[lineNumber, comments] = *it;
        if (comments.signatures.empty()) {
            return nullptr;
        }

        auto signatures = make_unique<parser::NodeVec>();
        auto signatureTranslator = rbs::SignatureTranslator(ctx);

        for (auto &signature : comments.signatures) {
            if (parser::isa_node<parser::DefMethod>(node) || parser::isa_node<parser::DefS>(node)) {
                auto sig = signatureTranslator.translateSignature(node, signature, comments.annotations);
                signatures->emplace_back(move(sig));
            } else if (auto send = parser::cast_node<parser::Send>(node)) {
                auto sig = signatureTranslator.translateType(send, signature, comments.annotations);
                signatures->emplace_back(move(sig));
            } else {
                Exception::raise("Unimplemented node type: {}", node->nodeName());
            }
        }

        methodSignatures.erase(it);
        return signatures;
    }
    return nullptr;
}

} // namespace

void SigsRewriter::checkForUnusedComments() {
    for (const auto &[lineNumber, comments] : methodSignatures) {
        for (const auto &sig : comments.signatures) {
            if (auto e = ctx.beginError(sig.typeLoc, core::errors::Rewriter::RBSUnusedComment)) {
                e.setHeader("Unused RBS signature comment. No method definition found after it");
            }
        }
    }
}

// Triggered once per file. We iterate over the source code and look for RBS comments
// Each RBS comment is stored in a hash map, methodSignatures. Key is the method name, value is a vector of Comments
void SigsRewriter::extractCommentsFromFile() {
    auto lines = absl::StrSplit(ctx.file.data(ctx).source(), '\n');
    Comments currentComments;
    uint32_t offset = 0;
    uint32_t lineNumber = 0;

    for (const auto &line : lines) {
        lineNumber++;
        auto [trimmedLine, leadingWhitespaceCount] = stripAsciiWhitespaceWithCount(line);

        // Empty lines between the RBS Comment and the method definition are allowed
        if (trimmedLine.empty()) {
            offset += line.length() + 1;
            continue;
        } else if (absl::StartsWith(trimmedLine, RBS_PREFIX)) {
            auto signature = trimmedLine.substr(RBS_PREFIX.size());
            uint32_t startOffset = offset + leadingWhitespaceCount;
            uint32_t endOffset = startOffset + trimmedLine.length();

            rbs::Comment comment{
                core::LocOffsets{startOffset, endOffset},
                core::LocOffsets{startOffset + 2, endOffset},
                signature,
            };
            currentComments.signatures.emplace_back(move(comment));
            offset += line.length() + 1;
            continue;
        } else if (absl::StartsWith(trimmedLine, ANNOTATION_PREFIX)) {
            auto annotation = trimmedLine.substr(ANNOTATION_PREFIX.size());
            uint32_t startOffset = offset + leadingWhitespaceCount;
            uint32_t endOffset = startOffset + trimmedLine.length();

            rbs::Comment comment{
                core::LocOffsets{startOffset, endOffset},
                core::LocOffsets{startOffset + 2, endOffset},
                annotation,
            };
            currentComments.annotations.emplace_back(move(comment));
            offset += line.length() + 1;
            continue;
        } else if (absl::StartsWith(trimmedLine, "#")) {
            offset += line.length() + 1;
            continue;
        }

        if (!currentComments.signatures.empty() || !currentComments.annotations.empty()) {
            methodSignatures[lineNumber] = move(currentComments);
        }

        // Clean up currentComments for next iteration
        currentComments = Comments{};
        offset += line.length() + 1;
    }
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
            if (auto signatures = signaturesForNode(ctx, target, methodSignatures)) {
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
        if (auto signatures = signaturesForNode(ctx, target, methodSignatures)) {
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
            result = move(node);
        },
        [&](parser::Class *klass) {
            klass->body = rewriteBody(move(klass->body));
            result = move(node);
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
            result = move(node);
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
    extractCommentsFromFile();
    auto body = rewriteBody(move(node));
    checkForUnusedComments();

    return body;
}

} // namespace sorbet::rbs
