#include "rewriter/RBSSignatures.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "rbs/MethodTypeTranslator.h"
#include "rbs/RBSParser.h"
#include "rbs/TypeTranslator.h"
#include <optional>

using namespace std;

namespace sorbet::rewriter {

/**
 * A collection of annotations and signatures comments found on a method definition.
 */
struct Comments {
    /**
     * RBS annotation comments found on a method definition.
     *
     * Annotations are formatted as `@some_annotation`.
     */
    std::vector<rbs::Comment> annotations;

    /**
     * RBS signature comments found on a method definition.
     *
     * Signatures are formatted as `#: () -> void`.
     */
    std::vector<rbs::Comment> signatures;
};

class RBSSignaturesWalk {
    // TODO: review and clean up
    Comments findRBSComments(string_view sourceCode, core::LocOffsets loc) {
        std::vector<rbs::Comment> annotations;
        std::vector<rbs::Comment> signatures;

        uint32_t beginIndex = loc.beginPos();

        // Everything in the file before the method definition
        string_view preDefinition = sourceCode.substr(0, sourceCode.rfind('\n', beginIndex));

        // Get all the lines before it
        std::vector<string_view> all_lines = absl::StrSplit(preDefinition, '\n');

        // We compute the current position in the source so we know the location of each comment
        uint32_t index = beginIndex;

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
                // Account for whitespace before comment e.g
                // # abc -> "abc"
                // #abc -> "abc"
                // int skip_after_hash = absl::StartsWith(line, "#: ") ? 3 : 2;

                int lineSize = line.size();
                auto rbsSignature = rbs::Comment{
                    core::LocOffsets{index, index + lineSize},
                    line.substr(line.find("#:") + 2),
                };
                signatures.insert(signatures.begin(), rbsSignature);
            }

            // Handle RDoc annotations `# @abstract`
            else if (absl::StartsWith(line, "# @")) {
                int lineSize = line.size();
                auto annotation = rbs::Comment{
                    core::LocOffsets{index, index + lineSize},
                    line.substr(line.find("# ") + 2),
                };
                annotations.insert(annotations.begin(), annotation);
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
    bool isVisibility(ast::Send *send) {
        if (!send->recv.isSelfReference()) {
            return false;
        }

        if (send->posArgs().size() != 1) {
            return false;
        }

        if (!ast::cast_tree<ast::MethodDef>(send->posArgs()[0])) {
            return false;
        }

        core::NameRef name = send->fun;
        return name == core::Names::public_() || name == core::Names::protected_() || name == core::Names::private_();
    }

    void transformMethodDef(core::MutableContext ctx, ast::ClassDef *classDef, ast::MethodDef *methodDef) {
        auto methodComments = findRBSComments(ctx.file.data(ctx).source(), methodDef->loc);

        for (auto &signature : methodComments.signatures) {
            if (auto rbsMethodType = rbs::RBSParser::parseSignature(ctx, signature)) {
                auto sig = rbs::MethodTypeTranslator::methodSignature(ctx, methodDef, std::move(rbsMethodType),
                                                                      methodComments.annotations);
                classDef->rhs.emplace_back(std::move(sig));
            }
        }
    }

    void transformAccessor(core::MutableContext ctx, ast::ClassDef *classDef, ast::Send *send) {
        auto attrComments = findRBSComments(ctx.file.data(ctx).source(), send->loc);

        for (auto &signature : attrComments.signatures) {
            if (auto rbsType = rbs::RBSParser::parseType(ctx, signature)) {
                auto sig = rbs::MethodTypeTranslator::attrSignature(ctx, send, std::move(rbsType));
                classDef->rhs.emplace_back(std::move(sig));
            }
        }
    }

public:
    RBSSignaturesWalk(core::MutableContext ctx) {}

    void preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto classDef = ast::cast_tree<ast::ClassDef>(tree);

        auto oldRHS = std::move(classDef->rhs);
        classDef->rhs.clear();
        classDef->rhs.reserve(oldRHS.size());

        for (auto &stat : oldRHS) {
            if (auto methodDef = ast::cast_tree<ast::MethodDef>(stat)) {
                transformMethodDef(ctx, classDef, methodDef);
            } else if (auto send = ast::cast_tree<ast::Send>(stat)) {
                if (isAccessor(send)) {
                    transformAccessor(ctx, classDef, send);
                } else if (isVisibility(send)) {
                    auto methodDef = ast::cast_tree<ast::MethodDef>(send->posArgs()[0]);
                    transformMethodDef(ctx, classDef, methodDef);
                }
            }

            classDef->rhs.emplace_back(std::move(stat));
        }
    }
};

ast::ExpressionPtr RBSSignatures::run(core::MutableContext ctx, ast::ExpressionPtr tree) {
    RBSSignaturesWalk rbs_translate(ctx);
    ast::TreeWalk::apply(ctx, rbs_translate, tree);

    return tree;
}
}; // namespace sorbet::rewriter
