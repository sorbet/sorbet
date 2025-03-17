#include "rewriter/RBSSignatures.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/rewriter.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

/**
 * A collection of annotations and signatures comments found on a method definition.
 */
struct Comments {
    /**
     * RBS annotation comments found on a method definition.
     *
     * Annotations are formatted as `@some_annotation`.
     */
    vector<rbs::Comment> annotations;

    /**
     * RBS signature comments found on a method definition.
     *
     * Signatures are formatted as `#: () -> void`.
     */
    vector<rbs::Comment> signatures;
};

class RBSSignaturesWalk {
    Comments findRBSComments(string_view sourceCode, core::LocOffsets loc) {
        vector<rbs::Comment> annotations;
        vector<rbs::Comment> signatures;

        uint32_t beginIndex = loc.beginPos();

        // Everything in the file before the method definition
        string_view preDefinition = sourceCode.substr(0, sourceCode.rfind('\n', beginIndex));

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
                auto rbsSignature = rbs::Comment{
                    core::LocOffsets{index, index + lineSize},
                    line.substr(2),
                };
                signatures.emplace_back(rbsSignature);
            }

            // Handle RDoc annotations `# @abstract`
            else if (absl::StartsWith(line, "# @")) {
                int lineSize = line.size();
                auto annotation = rbs::Comment{
                    core::LocOffsets{index, index + lineSize},
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

    // Check if the send is an accessor e.g `attr_reader`, `attr_writer`, `attr_accessor`
    bool isAccessor(ast::Send *send) {
        if (!send->recv.isSelfReference()) {
            return false;
        }

        core::NameRef name = send->fun;
        return name == core::Names::attrReader() || name == core::Names::attrWriter() ||
               name == core::Names::attrAccessor();
    }

    // Check if the send is a visibility modifier e.g `public`, `protected`, `private` before a method definition
    // and return the method definition if it is
    ast::MethodDef *asVisibilityWrappedMethod(ast::Send *send) {
        if (!send->recv.isSelfReference()) {
            return nullptr;
        }

        if (send->posArgs().size() != 1) {
            return nullptr;
        }

        if (!ast::cast_tree<ast::MethodDef>(send->getPosArg(0))) {
            return nullptr;
        }

        core::NameRef name = send->fun;
        if (name == core::Names::public_() || name == core::Names::protected_() || name == core::Names::private_() ||
            name == core::Names::privateClassMethod() || name == core::Names::publicClassMethod() ||
            name == core::Names::packagePrivate() || name == core::Names::packagePrivateClassMethod()) {
            return ast::cast_tree<ast::MethodDef>(send->getPosArg(0));
        }

        return nullptr;
    }

    vector<ast::ExpressionPtr> makeMethodDefSignatures(core::MutableContext ctx, ast::MethodDef *methodDef) {
        auto signatures = vector<ast::ExpressionPtr>();

        auto methodComments = findRBSComments(ctx.file.data(ctx).source(), methodDef->loc);
        auto signatureTranslator = rbs::SignatureTranslator(ctx);

        for (auto &signature : methodComments.signatures) {
            auto sig = signatureTranslator.translateSignature(methodDef, signature, methodComments.annotations);
            if (sig) {
                signatures.emplace_back(move(sig));
            }
        }

        return signatures;
    }

    vector<ast::ExpressionPtr> makeAccessorSignatures(core::MutableContext ctx, ast::Send *send) {
        auto signatures = vector<ast::ExpressionPtr>();
        auto attrComments = findRBSComments(ctx.file.data(ctx).source(), send->loc);
        auto signatureTranslator = rbs::SignatureTranslator(ctx);

        for (auto &signature : attrComments.signatures) {
            auto sig = signatureTranslator.translateType(send, signature, attrComments.annotations);
            if (sig) {
                signatures.emplace_back(move(sig));
            }
        }

        return signatures;
    }

public:
    RBSSignaturesWalk(core::MutableContext ctx) {}

    void postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        auto newRHS = ast::ClassDef::RHS_store();
        newRHS.reserve(classDef.rhs.size());

        for (auto &stat : classDef.rhs) {
            if (auto methodDef = ast::cast_tree<ast::MethodDef>(stat)) {
                auto signatures = makeMethodDefSignatures(ctx, methodDef);
                for (auto &signature : signatures) {
                    newRHS.emplace_back(move(signature));
                }
            } else if (auto send = ast::cast_tree<ast::Send>(stat)) {
                if (isAccessor(send)) {
                    auto signatures = makeAccessorSignatures(ctx, send);
                    for (auto &signature : signatures) {
                        newRHS.emplace_back(move(signature));
                    }
                } else if (auto methodDef = asVisibilityWrappedMethod(send)) {
                    auto signatures = makeMethodDefSignatures(ctx, methodDef);
                    for (auto &signature : signatures) {
                        newRHS.emplace_back(move(signature));
                    }
                }
            }

            newRHS.emplace_back(move(stat));
        }

        classDef.rhs = move(newRHS);
    }

    void postTransformBlock(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &block = ast::cast_tree_nonnull<ast::Block>(tree);

        if (auto methodDef = ast::cast_tree<ast::MethodDef>(block.body)) {
            auto signatures = makeMethodDefSignatures(ctx, methodDef);

            if (signatures.empty()) {
                return;
            }

            auto newBody = ast::InsSeq::STATS_store();
            newBody.reserve(signatures.size());

            for (auto &signature : signatures) {
                newBody.emplace_back(move(signature));
            }

            block.body = ast::MK::InsSeq(block.loc, move(newBody), move(block.body));

            return;
        }

        if (auto body = ast::cast_tree<ast::InsSeq>(block.body)) {
            auto newBody = ast::InsSeq::STATS_store();

            for (auto &stat : body->stats) {
                if (auto methodDef = ast::cast_tree<ast::MethodDef>(stat)) {
                    auto signatures = makeMethodDefSignatures(ctx, methodDef);
                    for (auto &signature : signatures) {
                        newBody.emplace_back(move(signature));
                    }
                }

                newBody.emplace_back(move(stat));
            }

            if (auto methodDef = ast::cast_tree<ast::MethodDef>(body->expr)) {
                auto signatures = makeMethodDefSignatures(ctx, methodDef);
                for (auto &signature : signatures) {
                    newBody.emplace_back(move(signature));
                }
            }

            body->stats = move(newBody);

            return;
        }
    }
};

} // namespace

ast::ExpressionPtr RBSSignatures::run(core::MutableContext ctx, ast::ExpressionPtr tree) {
    RBSSignaturesWalk rbsTranslate(ctx);
    ast::TreeWalk::apply(ctx, rbsTranslate, tree);

    return tree;
}

}; // namespace sorbet::rewriter
