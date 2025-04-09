#include "rbs/CommentsAssociator.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rbs {

const std::string_view CommentsAssociator::RBS_PREFIX = "#:";
const std::string_view CommentsAssociator::ANNOTATION_PREFIX = "# @";

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
}; // namespace

void CommentsAssociator::consumePrecedingComments(parser::Node *node, const string_view &prefix) {
    auto nodeStartLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;

    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        if (it->first < nodeStartLine) {
            if (absl::StartsWith(it->second.string, prefix)) {
                if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                    e.setHeader("Unused RBS signature comment. No method definition found after it");
                }
            }
            it = commentByLine.erase(it);
        } else {
            break;
        }
    }
}

void CommentsAssociator::consumeDanglingComments(parser::Node *node) {
    auto nodeEndLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        if (it->first < nodeEndLine) {
            if (absl::StartsWith(it->second.string, RBS_PREFIX)) {
                if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                    e.setHeader("Unused RBS signature comment. No method definition found after it");
                }
            }
            it = commentByLine.erase(it);
        } else {
            break;
        }
    }
}

void CommentsAssociator::associateCommentsToLines() {
    for (auto &[start, end] : commentLocations) {
        auto comment_string = ctx.file.data(ctx).source().substr(start, end - start);
        auto start32 = static_cast<uint32_t>(start);
        auto end32 = static_cast<uint32_t>(end);
        auto comment = CommentNode{core::LocOffsets{start32, end32}, comment_string};

        auto line = core::Loc::pos2Detail(ctx.file.data(ctx), start32).line;
        commentByLine[line] = comment;
    }
}

void CommentsAssociator::associateCommentsToNode(parser::Node *node,
                                                 const InlinedVector<std::string_view, 2> &prefixes) {
    auto nodeStartLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;

    vector<CommentNode> comments;

    // fmt::print("associating comments to {}\n", node->toString(ctx));
    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        // fmt::print("it->first: {}, nodeStartLine: {}\n", it->first, nodeStartLine);
        if (it->first < nodeStartLine) {
            bool found = false;
            for (const auto &prefix : prefixes) {
                if (absl::StartsWith(it->second.string, prefix)) {
                    // fmt::print("associated {} to {}\n", it->second.string, node->toString(ctx));
                    comments.emplace_back(it->second);
                    it = commentByLine.erase(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
                it++;
            }
        } else {
            break;
        }
    }

    commentsByNode[node] = comments;
}

void CommentsAssociator::walkNodes(parser::Node *node) {
    if (node == nullptr) {
        return;
    }

    typecase(
        node,

        // Blocks
        [&](parser::Begin *begin) {
            // @kaan begin node also shows up in nested classes, we don't want to associate comments to it
            // associateCommentsToNode(node);

            for (auto &stmt : begin->stmts) {
                walkNodes(stmt.get());
            }
            consumeDanglingComments(node);
        },
        [&](parser::Kwbegin *kwbegin) {
            consumePrecedingComments(node, RBS_PREFIX);
            for (auto &stmt : kwbegin->stmts) {
                walkNodes(stmt.get());
            }
            consumeDanglingComments(node);
        },

        // Classes
        [&](parser::Class *cls) {
            associateCommentsToNode(node, {ANNOTATION_PREFIX}); // @kaan associate annotations to error later
            consumePrecedingComments(node, RBS_PREFIX);         // @kaan consume and error RBS comments

            if (auto body = cls->body.get()) {
                walkNodes(body);
            }
            consumeDanglingComments(node);
        },
        [&](parser::SClass *sclass) {
            associateCommentsToNode(node, {ANNOTATION_PREFIX});
            consumePrecedingComments(node, RBS_PREFIX);

            if (auto body = sclass->body.get()) {
                walkNodes(body);
            }
            consumeDanglingComments(node);
        },

        // Modules
        [&](parser::Module *mod) {
            associateCommentsToNode(node, {ANNOTATION_PREFIX});
            consumePrecedingComments(node, RBS_PREFIX);

            if (auto body = mod->body.get()) {
                walkNodes(body);
            }

            consumeDanglingComments(node);
        },
        [&](parser::DefMethod *def) {
            associateCommentsToNode(node);
            walkNodes(def->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::DefS *def) {
            associateCommentsToNode(node);
            walkNodes(def->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::Send *send) {
            if (isVisibilitySend(send)) {
                associateCommentsToNode(send->args[0].get());
                consumeDanglingComments(send);
            } else if (isAttrAccessorSend(send)) {
                associateCommentsToNode(send);
                consumeDanglingComments(send);
            }
        },
        [&](parser::Assign *assign) {
            if (auto rhs = assign->rhs.get()) {
                walkNodes(rhs);
            }
            consumeDanglingComments(assign);
        },
        [&](parser::AndAsgn *andAsgn) {
            if (auto rhs = andAsgn->right.get()) {
                walkNodes(rhs);
            }
            consumeDanglingComments(andAsgn);
        },
        [&](parser::OrAsgn *orAsgn) {
            if (auto rhs = orAsgn->right.get()) {
                walkNodes(rhs);
            }
            consumeDanglingComments(orAsgn);
        },
        [&](parser::OpAsgn *opAsgn) {
            if (auto rhs = opAsgn->right.get()) {
                walkNodes(rhs);
            }
            consumeDanglingComments(opAsgn);
        },
        [&](parser::Masgn *masgn) {
            walkNodes(masgn->rhs.get());
            consumeDanglingComments(masgn);
        },
        [&](parser::Block *block) {
            consumePrecedingComments(node, RBS_PREFIX);
            walkNodes(block->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::For *forNode) {
            walkNodes(forNode->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::Array *array) {
            for (auto &elem : array->elts) {
                walkNodes(elem.get());
            }
            consumeDanglingComments(node);
        },
        [&](parser::Rescue *rescue) {
            walkNodes(rescue->body.get());
            for (auto &rescued : rescue->rescue) {
                walkNodes(rescued.get());
            }
            walkNodes(rescue->else_.get());
            consumeDanglingComments(node);
        },
        [&](parser::Ensure *ensure) {
            walkNodes(ensure->body.get());
            walkNodes(ensure->ensure.get());
            consumeDanglingComments(node);
        },
        [&](parser::If *if_) {
            walkNodes(if_->then_.get());
            walkNodes(if_->else_.get());
            consumeDanglingComments(node);
        },
        [&](parser::Resbody *resbody) {
            walkNodes(resbody->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::Case *case_) {
            for (auto &when : case_->whens) {
                walkNodes(when.get());
            }
            walkNodes(case_->else_.get());
            consumeDanglingComments(node);
        },
        [&](parser::When *when) {
            walkNodes(when->body.get());
            consumeDanglingComments(node);
        },
        [&](parser::Node *other) {
            // Do nothing
        });
}

void CommentsAssociator::run(unique_ptr<parser::Node> &node) {
    associateCommentsToLines();
    walkNodes(node.get());

    // for (auto &[node, comments] : commentsByNode) {
    //     fmt::print("Node {}: {}\n", node->toString(ctx), comments.size());
    //     for (auto &comment : comments) {
    //         fmt::print("  {}\n", comment.string);
    //     }
    //     fmt::print("-----\n\n");
    // }
};

} // namespace sorbet::rbs
