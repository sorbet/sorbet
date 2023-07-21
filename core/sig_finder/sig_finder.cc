#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

#include "core/sig_finder/sig_finder.h"

using namespace std;

namespace sorbet::sig_finder {

namespace {

// parseSigTop usually expects to be called when `ctx` is a class. The sig_finder code
// sometimes runs after class_flatten, when sigs have been moved inside <static-init>
// methods. We have to put the owner back to what it would have looked like at the top-level
core::SymbolRef getEffectiveOwner(core::Context ctx) {
    if (ctx.owner.isClassOrModule()) {
        // class_flatten hasn't run, this `sig` send is still at a class top-level
        return ctx.owner;
    } else {
        auto methodOwner = ctx.owner.asMethodRef();
        ENFORCE(methodOwner.data(ctx)->name == core::Names::staticInit());
        auto owner = methodOwner.data(ctx)->owner.data(ctx)->attachedClass(ctx);
        ENFORCE(owner.exists());
        return owner;
    }
}

} // namespace

void SigFinder::preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
    auto loc = core::Loc(ctx.file, tree.loc());

    if (!this->narrowestClassDefRange.exists()) {
        // No narrowestClassDefRange yet, so take the loc of the first ClassDef we see
        // Usually this is the `<root>` class (whole file), but sometimes the caller might provide
        // us a specific ClassDef to look in if it has one (not necessarily root)
        this->narrowestClassDefRange = loc;
    } else if (loc.contains(this->queryLoc) && this->narrowestClassDefRange.contains(loc)) {
        // `loc` is contained in the current narrowestClassDefRange, and still contains `queryLoc`
        this->narrowestClassDefRange = loc;

        if (this->result_.has_value() && !loc.contains(ctx.locAt(this->result_->origSend->loc))) {
            // If there's a result and it's not contained in the new narrowest range, we have to toss it out
            // (Method defs and class defs are not necessarily sorted by their locs)
            this->result_ = nullopt;
        }
    }

    this->scopeContainsQueryLoc.emplace_back(loc.contains(this->queryLoc));
}

void SigFinder::postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
    ENFORCE(!this->scopeContainsQueryLoc.empty());
    this->scopeContainsQueryLoc.pop_back();
}

void SigFinder::preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
    if (this->result_.has_value()) {
        if (this->result_->origSend->loc.endPos() <= tree.loc().beginPos() &&
            tree.loc().endPos() <= queryLoc.beginPos()) {
            // There is a method definition between the current result sig and the queryLoc,
            // so the sig we found is not for the right method.
            this->result_ = nullopt;
        }
    }
}

void SigFinder::postTransformRuntimeMethodDefinition(core::Context ctx, ast::ExpressionPtr &tree) {
    if (this->result_.has_value()) {
        if (this->result_->origSend->loc.endPos() <= tree.loc().beginPos() &&
            tree.loc().endPos() <= queryLoc.beginPos()) {
            // There is a method definition between the current result sig and the queryLoc,
            // so the sig we found is not for the right method.
            this->result_ = nullopt;
        }
    }
}

void SigFinder::preTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
    auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

    if (!resolver::TypeSyntax::isSig(ctx, send)) {
        return;
    }

    ENFORCE(!this->scopeContainsQueryLoc.empty());
    if (!this->scopeContainsQueryLoc.back()) {
        // Regardless of whether this send is after the queryLoc or inside the narrowestClassDefRange,
        // we're in a ClassDef whose scope doesn't contain the queryLoc.
        // (one case where this happens: nested Inner class)
        return;
    }

    auto currentLoc = ctx.locAt(tree.loc());
    if (!currentLoc.exists()) {
        // Defensive in case location information is disabled (e.g., certain fuzzer modes)
        return;
    }

    ENFORCE(this->narrowestClassDefRange.exists());

    if (!this->narrowestClassDefRange.contains(currentLoc)) {
        // This send occurs outside the current narrowest range we know of for a ClassDef that
        // still contains queryLoc, so even if this Send is after the queryLoc, it would not be
        // in the right scope.
        return;
    } else if (!(currentLoc.endPos() <= this->queryLoc.beginPos())) {
        // Query loc is not after the send
        return;
    }

    if (this->result_.has_value()) {
        // Method defs are not guaranteed to be sorted in order by their declLocs
        auto resultLoc = this->result_->origSend->loc;
        if (resultLoc.beginPos() < currentLoc.beginPos()) {
            // Found a method defined before the query but later than previous result: overwrite previous result
            auto owner = getEffectiveOwner(ctx);
            auto parsedSig = resolver::TypeSyntax::parseSigTop(ctx.withOwner(owner), send, core::Symbols::untyped());
            this->result_ = make_optional<resolver::ParsedSig>(move(parsedSig));
        } else {
            // We've already found an earlier result, so the current is not the first
        }
    } else {
        // Haven't found a result yet, so this one is the best so far.
        auto owner = getEffectiveOwner(ctx);
        auto parsedSig = resolver::TypeSyntax::parseSigTop(ctx.withOwner(owner), send, core::Symbols::untyped());

        this->result_ = make_optional<resolver::ParsedSig>(move(parsedSig));
    }
}

optional<resolver::ParsedSig> SigFinder::findSignature(core::Context ctx, ast::ExpressionPtr &tree,
                                                       core::Loc queryLoc) {
    SigFinder sigFinder(queryLoc);
    ast::TreeWalk::apply(ctx, sigFinder, tree);
    return move(sigFinder.result_);
}

} // namespace sorbet::sig_finder
