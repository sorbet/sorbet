#include "ast/treemap/treemap.h"

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

struct SigFinderTraversal {
    const core::Loc queryLoc;

    // Track the narrowest location range that still contains the queryLoc.
    //
    // If we find a method that's after queryLoc but it's not in this narrowest range,
    // it means we found a sig that's outside the scope where the queryLoc was.
    core::Loc narrowestClassDefRange;

    // Track whether the current scope has the queryLoc.
    std::vector<bool> scopeContainsQueryLoc;

    // The best `sig` send found so far, and the owner it should be parsed in (i.e., a singleton
    // class, for a `sig` on a `self.` method). We deliberately do NOT parse this send as we find
    // it: parsing a `sig` has the side effect of reporting type errors on it, so it must only
    // ever be parsed once, for the single winning candidate, and only if the caller actually asked
    // for the parsed result (see parseBestSend, findSignature, and findSignatureSend below).
    const ast::Send *bestSend_;
    core::SymbolRef bestOwner_;

    SigFinderTraversal(core::Loc queryLoc)
        : queryLoc(queryLoc), narrowestClassDefRange(core::Loc::none()), scopeContainsQueryLoc(std::vector<bool>{}),
          bestSend_(nullptr) {}

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &tree) {
        auto loc = ctx.locAt(tree.loc);

        if (!this->narrowestClassDefRange.exists()) {
            // No narrowestClassDefRange yet, so take the loc of the first ClassDef we see
            // Usually this is the `<root>` class (whole file), but sometimes the caller might provide
            // us a specific ClassDef to look in if it has one (not necessarily root)
            this->narrowestClassDefRange = loc;
        } else if (loc.contains(this->queryLoc) && this->narrowestClassDefRange.contains(loc)) {
            // `loc` is contained in the current narrowestClassDefRange, and still contains `queryLoc`
            this->narrowestClassDefRange = loc;

            if (this->bestSend_ != nullptr && !loc.contains(ctx.locAt(this->bestSend_->loc))) {
                // If there's a result and it's not contained in the new narrowest range, we have to toss it out
                // (Method defs and class defs are not necessarily sorted by their locs)
                this->bestSend_ = nullptr;
            }
        }

        this->scopeContainsQueryLoc.emplace_back(loc.contains(this->queryLoc));
    }

    void postTransformClassDef(core::Context ctx, const ast::ClassDef &tree) {
        ENFORCE(!this->scopeContainsQueryLoc.empty());
        this->scopeContainsQueryLoc.pop_back();
    }

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &tree) {
        if (this->bestSend_ != nullptr) {
            if (this->bestSend_->loc.endPos() <= tree.loc.beginPos() && tree.loc.endPos() <= queryLoc.beginPos()) {
                // There is a method definition between the current result sig and the queryLoc,
                // so the sig we found is not for the right method.
                this->bestSend_ = nullptr;
            }
        }
    }

    void postTransformRuntimeMethodDefinition(core::Context ctx, const ast::RuntimeMethodDefinition &tree) {
        if (this->bestSend_ != nullptr) {
            if (this->bestSend_->loc.endPos() <= tree.loc.beginPos() && tree.loc.endPos() <= queryLoc.beginPos()) {
                // There is a method definition between the current result sig and the queryLoc,
                // so the sig we found is not for the right method.
                this->bestSend_ = nullptr;
            }
        }
    }

    void preTransformSend(core::Context ctx, const ast::Send &send) {
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

        auto currentLoc = ctx.locAt(send.loc);
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

        // Note: we deliberately do not parse `send` here. Parsing a `sig` has the side effect of
        // reporting type errors on it, and this loop may pass over the same `sig` (or an unrelated,
        // syntactically-invalid one) many times while searching for the best candidate. We only ever
        // want to parse the single, final winning candidate, and only when the caller actually asked
        // for the parsed result--see parseBestSend, findSignature, and findSignatureSend below.
        if (this->bestSend_ == nullptr || this->bestSend_->loc.beginPos() < currentLoc.beginPos()) {
            // Either we haven't found a candidate yet, or this send is a method defined before the
            // query but later than the previous candidate: replace the previous candidate.
            this->bestSend_ = &send;
            this->bestOwner_ = getEffectiveOwner(ctx);
        } else {
            // We've already found a later candidate, so the current send is not the best one.
        }
    }

    optional<SigFinder::Result> parseBestSend(core::Context ctx) const {
        if (this->bestSend_ == nullptr) {
            return nullopt;
        }
        auto parsedSig = resolver::TypeSyntax::parseSigTop(ctx.withOwner(this->bestOwner_), *this->bestSend_,
                                                            core::Symbols::untyped());
        return SigFinder::Result(move(parsedSig), *this->bestSend_);
    }
};

} // namespace

optional<SigFinder::Result> SigFinder::findSignature(core::Context ctx, const ast::ExpressionPtr &tree,
                                                     core::Loc queryLoc) {
    SigFinderTraversal sigFinder(queryLoc);
    ast::ConstTreeWalk::apply(ctx, sigFinder, tree);
    return sigFinder.parseBestSend(ctx);
}
optional<SigFinder::Result> SigFinder::findSignature(core::Context ctx, const ast::ClassDef &tree, core::Loc queryLoc) {
    SigFinderTraversal sigFinder(queryLoc);
    ast::ConstTreeWalk::apply(ctx, sigFinder, tree);
    return sigFinder.parseBestSend(ctx);
}

const ast::Send *SigFinder::findSignatureSend(core::Context ctx, const ast::ExpressionPtr &tree, core::Loc queryLoc) {
    SigFinderTraversal sigFinder(queryLoc);
    ast::ConstTreeWalk::apply(ctx, sigFinder, tree);
    return sigFinder.bestSend_;
}
const ast::Send *SigFinder::findSignatureSend(core::Context ctx, const ast::ClassDef &tree, core::Loc queryLoc) {
    SigFinderTraversal sigFinder(queryLoc);
    ast::ConstTreeWalk::apply(ctx, sigFinder, tree);
    return sigFinder.bestSend_;
}

} // namespace sorbet::sig_finder
