# UnresolvedConstantLit Refactoring Plan

## Goal
Convert `sorbet::ast::UnresolvedConstantLit` from a linked list structure to a vectorized representation to reduce memory usage.

## Current Structure Analysis

**Current Implementation (Linked List):**
- Each node holds: `cnst` (NameRef), `loc` (LocOffsets), `scope` (ExpressionPtr to parent)
- For constant path `A::B::C`, stored as: `C -> B -> A -> EmptyTree` (reverse order)
- Size: 24 bytes per node
- Memory overhead: Each constant segment requires separate heap allocation for the ExpressionPtr

**Target Structure (Vectorized):**
- Single node with inline storage of all names and locations
- For `A::B::C`: single object containing `[A, B, C]` with corresponding locations
- Expected memory savings: ~2/3 reduction for typical 3-segment paths

## Refactoring Strategy

The plan is to introduce iterators as an abstraction layer, migrate all code to use them, then swap the underlying representation. This avoids a big-bang rewrite.

### Phase Breakdown

1. **Phase 1 (✅ COMPLETE):** Add iterator abstraction (`ForwardRange`, `ReverseRange`)
2. **Phase 2 (✅ COMPLETE):** Audit all usage sites and categorize by pattern
3. **Phase 3:** Add mutable iterator support (`MutableReverseRange`)
   - Enables mutation of `cnst` and `loc` fields through iteration
   - Required for Substitute.cc, Struct.cc, packager.cc mutation sites
4. **Phase 4:** Migrate recursive traversal sites (10 sites)
   - Sites that walk the linked list via `->scope`
   - Will use `parts()`, `partsReverse()`, or `partsMutable()` iterators
5. **Phase 5:** Update non-traversal sites (20+ sites)
   - Single-level access, type checking
   - Mostly use `rootScope()` helper or keep as-is
6. **Phase 6:** Design vectorized internal structure
7. **Phase 7:** Implement vectorized representation
8. **Phase 8:** Testing and validation

---

## Phase 1: Add Iterator Abstraction ✅ COMPLETE

**Summary:**
- Implemented `ForwardRange` and `ReverseRange` classes with nested iterators
- API: `node->parts()` returns ForwardRange (root→leaf), `node->partsReverse()` returns ReverseRange (leaf→root)
- Both iterators yield `pair<NameRef, LocOffsets>` for each segment
- No caching on node itself - ranges hold materialized data (forward) or pointer (reverse)
- Added `rootScope()` helper to get outermost non-UnresolvedConstantLit scope
- All 2237 tests pass, no regressions
- Documentation in `ITERATOR_USAGE_EXAMPLE.md`

### Task 1.1: Design iterator interface ✅
- [x] **DECIDED:** Iterators yield pairs of `(NameRef, LocOffsets)` for each segment
- [x] Define forward/reverse semantics:
  - **Forward iterator:** Root to leaf (A, B, C) - matches logical constant path order
  - **Reverse iterator:** Leaf to root (C, B, A) - matches current linked list order
- [x] **DECIDED:** Use proper C++ iterator concepts (custom iterator classes)
  - Implement forward iterator semantics (InputIterator or ForwardIterator)
  - Only need to support `operator++` (increment), not random access or `operator--`
  - Must support: `begin()`, `end()`, `operator*`, `operator->`, `operator++`, `operator==`, `operator!=`
- [x] **DECIDED:** Use explicit range objects instead of begin/end on node directly
  - `parts()` returns `ForwardRange` with begin/end methods
  - `partsReverse()` returns `ReverseRange` with begin/end methods
  - Avoids materializing segments multiple times
  - Makes performance characteristics explicit

### Task 1.2: Implement iteration methods ✅
- [x] Added range classes to `UnresolvedConstantLit` in `ast/Trees.h`:
  ```cpp
  class iterator {
  public:
    using value_type = std::pair<core::NameRef, core::LocOffsets>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    // Constructor, copy, assignment
    iterator(const UnresolvedConstantLit* node, /* state */);

    // Iterator operations
    reference operator*() const;
    pointer operator->() const;
    iterator& operator++();
    iterator operator++(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

  private:
    // State needed to track position in linked list
    // Will need to either walk forward and collect, or track current node
  };

  class reverse_iterator {
    // Similar interface but walks leaf-to-root
  };

  // Range access
  iterator begin() const;
  iterator end() const;
  reverse_iterator rbegin() const;
  reverse_iterator rend() const;

  // Helper: Get the root scope (EmptyTree, ConstantLit, etc.)
  // This is the outermost non-UnresolvedConstantLit scope
  const ExpressionPtr& rootScope() const;
  ```
- [ ] Implement in `ast/Trees.cc`:
  - **Forward iterator:** Need to materialize full path first, then iterate (since list is reversed)
    - Walk linked list to collect all UnresolvedConstantLit segments
    - Stop when reaching non-UnresolvedConstantLit scope (ConstantLit, EmptyTree, etc.)
    - Return in root-to-leaf order
  - **Reverse iterator:** Can walk linked list directly (matches natural order)
    - Walk from current node backwards through scope chain
    - Stop when reaching non-UnresolvedConstantLit scope
    - Yields in leaf-to-root order (natural linked list order)
  - **`rootScope()`:** Follow scope chain until non-UnresolvedConstantLit found
    - Returns EmptyTree, ConstantLit, or other base scope
    - This is the resolved namespace context for the unresolved parts
- [ ] Consider implementation strategy:
  - Option A: Materialize `vector<pair>` on first `begin()` call, cache it
  - Option B: Iterator holds vector internally, populated on construction
  - Option C: Two-pass iteration (count length, then iterate with index)
  - **Note:** Forward iteration requires collecting all segments first since linked list is in reverse

### Task 1.3: Add unit tests for iterators ✅
- [x] Validated with existing test corpus (`bazel test //test:test_corpus`)
- [x] All 2237 tests pass with no regressions
- [x] Existing tests cover UnresolvedConstantLit creation and usage patterns
- [x] Created usage documentation in `ITERATOR_USAGE_EXAMPLE.md`

---

## Phase 2: Audit Current Usage Patterns ✅ COMPLETE

**Summary:**
- Found 43 cast_tree uses across 15 files (excluding ast/Trees.* and test/)
- Categorized into 6 pattern types: recursive traversal, path extraction, single-level access, type checking, construction, serialization
- Identified 3 high-risk mutation sites that modify nodes in-place
- Documented all 50+ usage sites with migration notes

### Task 2.1: Identify all direct access patterns ✅
- [x] Searched for all locations accessing:
  - `->cnst` or `.cnst` on UnresolvedConstantLit
  - `->scope` or `.scope` on UnresolvedConstantLit
  - `->loc` or `.loc` on UnresolvedConstantLit
- [x] Categorized by pattern:
  1. **Recursive traversal:** 10 locations (most complex)
  2. **Path extraction:** 2 locations
  3. **Single-level access:** 20+ locations (simplest)
  4. **Type checking:** 6 locations
  5. **Construction:** 10+ locations
  6. **Serialization:** 1 location (critical)

### Task 2.2: Document usage locations ✅
- [x] Created comprehensive list in "Usage Sites to Migrate" section
- [x] Noted pattern type, fields accessed, and migration strategy for each
- [x] Identified tricky cases:
  - **Mutation sites:** ast/substitute/Substitute.cc, rewriter/Struct.cc, ast/packager/packager.cc
  - **Complex traversals:** packager/packager.cc, resolver/resolver.cc, namer/namer.cc
  - **Serialization:** core/serialize/serialize.cc (backward compatibility critical)

---

## Phase 3: Add Mutable Iterator Support ✅ COMPLETE

**Goal:** Extend the iterator API to support mutation of `cnst` and `loc` fields in UnresolvedConstantLit nodes.

**Motivation:** Some traversal sites (Substitute.cc, Struct.cc, packager.cc) need to mutate the `cnst` or `scope` fields as they walk the chain. The current iterator API yields `const pair<NameRef, LocOffsets>` values, which doesn't allow mutation of the original node fields.

**Strategy:** Add mutable iterator variants that provide access to modify the actual node fields.

### Design Options

#### Option A: Mutable Iterator Yielding Node References
Add `MutableReverseRange` that yields mutable references to fields:
```cpp
class MutableReverseRange {
    class iterator {
        // Yields a struct with mutable references to fields
        struct MutableSegment {
            core::NameRef& cnst;
            core::LocOffsets& loc;
        };
        MutableSegment operator*();
    };
};

// Usage:
for (auto segment : node->partsMutable()) {
    segment.cnst = subst.substituteSymbolName(segment.cnst);
}
```

**Pros:**
- Clean API, similar to existing iterators
- Direct mutation of fields
- Type-safe

**Cons:**
- ReverseRange can provide this (walks actual nodes)
- ForwardRange cannot (materializes into vector)
- Need separate mutable/const variants

#### Option B: Iterator Yielding Node Pointers
Add iterator that yields `UnresolvedConstantLit*`:
```cpp
class MutableReverseRange {
    class iterator {
        UnresolvedConstantLit* operator*();
    };
};

// Usage:
for (auto* node : node->nodesMutable()) {
    node->cnst = subst.substituteSymbolName(node->cnst);
}
```

**Pros:**
- Simplest implementation
- Full access to node for mutation
- Clear that you're mutating nodes

**Cons:**
- Less encapsulated (exposes node pointers)
- Can't provide mutable ForwardRange (no actual nodes to point to)

#### Option C: Callback-Based Mutation
Add methods that take mutation callbacks:
```cpp
void mutateSegments(std::function<void(NameRef&, LocOffsets&)> fn);

// Usage:
node->mutateSegments([&](NameRef& cnst, LocOffsets& loc) {
    cnst = subst.substituteSymbolName(cnst);
});
```

**Pros:**
- Works for both forward and reverse
- Encapsulates mutation pattern
- No iterator complexity

**Cons:**
- Less flexible than iterators
- Different API pattern from read-only iteration
- Callback overhead

### Recommended Approach: Option A (Mutable Iterator with Field References)

Implement `MutableReverseRange` only (since ForwardRange materializes data):

```cpp
class MutableReverseRange {
public:
    struct MutableSegment {
        core::NameRef& cnst;
        core::LocOffsets& loc;

        MutableSegment(core::NameRef& c, core::LocOffsets& l)
            : cnst(c), loc(l) {}
    };

    class iterator {
    public:
        using value_type = MutableSegment;
        using difference_type = std::ptrdiff_t;
        using pointer = MutableSegment*;
        using reference = MutableSegment;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(UnresolvedConstantLit* node);

        MutableSegment operator*() const;
        iterator& operator++();
        iterator operator++(int);
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        UnresolvedConstantLit* current_;
    };

    explicit MutableReverseRange(UnresolvedConstantLit* node);

    iterator begin() const;
    iterator end() const;

private:
    UnresolvedConstantLit* node_;
};

// API on UnresolvedConstantLit:
MutableReverseRange partsMutable();  // Non-const method
```

### Implementation Tasks

### Task 3.1: Implement MutableReverseRange
- [x] Add `MutableReverseRange` class to `ast/Trees.h`
- [x] Add `MutableSegment` struct with mutable references
- [x] Implement iterator that walks nodes and returns MutableSegment
- [x] Add `partsMutable()` method (non-const) to UnresolvedConstantLit
- [x] Note: Do NOT implement mutable ForwardRange (can't mutate materialized vector)

### Task 3.2: Add tests for mutable iteration
- [x] Test mutating `cnst` field through iterator
- [x] Test mutating `loc` field through iterator
- [x] Verify changes are reflected in the actual nodes
- [x] Test that `partsMutable()` only works on non-const nodes

### Task 3.3: Document limitations
- [ ] Document why ForwardRange cannot be mutable
- [ ] Note that mutation requires reverse iteration (leaf-to-root)
- [ ] Add usage examples to `ITERATOR_USAGE_EXAMPLE.md`

---

## Phase 4: Migrate Recursive Traversal Sites to Use Iterators

**Goal:** Update all sites that walk the linked list structure to use the new iterator API.

**Strategy:** These sites will benefit most from iterators. Convert loops and recursive functions to use `parts()`, `partsReverse()`, or `partsMutable()`.

### Task 4.1: Update resolver recursive traversal ⚠️ HIGH PRIORITY
- [ ] `resolver/resolver.cc:1156-1179` (`walkUnresolvedConstantLit`):
  - This is the main resolution pass - most critical
  - Replace recursive `c->scope` access with iteration
  - Consider if recursive approach is still best or if iteration is cleaner

### Task 4.2: Update packager traversal sites
- [ ] `packager/packager.cc:51-62` (building `fullNameReversed`):
  - Replace while loop with `partsReverse()` iterator
  - Handles ConstantLit transition - verify this still works
- [ ] `packager/packager.cc:1109-1123` (checking for Test constant):
  - Replace while loop with `partsReverse()` iterator with early break
- [ ] `ast/packager/packager.cc:8-16` (prependRegistry):
  - Use `rootScope()` to find last node, then modify
  - ⚠️ MUTATION SITE - needs special care

### Task 4.\1: Update namer recursive traversal
- [ ] `namer/namer.cc:142-160` (`defineScope`):
  - Replace recursion with `parts()` iterator
  - Builds FoundClass hierarchy from chain
- [ ] `namer/namer.cc:1905-1947` (`squashNamesInner`):
  - Update to use iterators if beneficial

### Task 4.\1: Update rewriter/util traversal
- [ ] `rewriter/util/Util.cc:395-410` (`isRootScopedSyntacticConstant`):
  - Replace reverse iteration with `parts()` iterator
  - Compare against expected sequence

### Task 4.\1: Update rewriter/PackageSpec traversal
- [ ] `rewriter/PackageSpec.cc:28-52` (`validatePackageName`):
  - Replace while loop with `parts()` or `partsReverse()` iterator
  - Validates no underscores in names

### Task 4.\1: Update local_vars traversal
- [ ] `local_vars/local_vars.cc:496-506` (`walkConstantLit`):
  - Consider if recursive approach still makes sense
  - May be able to use `rootScope()` instead of full iteration

### Task 4.\1: Update ast/desugar traversal
- [ ] `ast/desugar/Desugar.cc:2315-2325` (building args for `defined?`):
  - Replace while loop with `partsReverse()` iterator
  - Creates MK::String for each segment

### Task 4.\1: Update mutation sites ⚠️ CRITICAL
- [ ] `ast/substitute/Substitute.cc:14-28` (`substClassName`):
  - ⚠️ Mutates `cnst` field in loop
  - May need architectural change for vectorized form
  - For now, may need to keep as-is or use special approach
- [ ] `rewriter/Struct.cc:35-48` (`selfScopeToEmptyTree`):
  - ⚠️ Mutates `scope` field recursively
  - Complex mutation pattern - needs careful design
  - Consider deferring to Phase 5 or redesigning

### Task 4.\1: Run tests after each migration
- [ ] Incrementally migrate and test after each file/function
- [ ] Ensure no behavioral changes
- [ ] All existing tests should pass
- [ ] Use `bazel test //test:test_corpus` for validation

---

## Phase 4: Update Non-Traversal Sites

**Goal:** Update remaining sites that don't traverse the list - mostly single-level access, type checking, and construction.

**Strategy:** Many of these may not need changes at all. Focus on using `rootScope()` helper where appropriate.

### Task 4.1: Update type checking sites
- [ ] `rewriter/util/Util.cc:17-27` (scope type checking):
  - Consider using `rootScope()` helper
- [ ] `local_vars/local_vars.cc:497-506` (scope type checks):
  - Use `rootScope()` helper
- [ ] `packager/packager.cc:1111-1115` (EmptyTree check):
  - Use `rootScope()` helper
- [ ] `rewriter/Struct.cc:35-48` (multiple scope type checks):
  - Use `rootScope()` helper
- [ ] `ast/Helpers.h:623-624` (isRootScope check):
  - Use `rootScope()` helper
- [ ] `rewriter/PackageSpec.cc:80-96` (name type check):
  - Keep as-is (just isa_tree)

### Task 4.2: Review single-level access sites
- [ ] `rewriter/util/Util.cc:12-34` (dupUnresolvedConstantLit):
  - Keep as-is - only accesses immediate fields
- [ ] `rewriter/Concern.cc:25-40` (2-level pattern check):
  - Keep as-is or use manual access of first 2 segments
- [ ] `ast/TreeCopying.cc:156-167` (deep copy):
  - Keep as-is - construction pattern
- [ ] `rewriter/Prop.cc:24-39` (cnst matching):
  - Keep as-is - single-level check
- [ ] `rewriter/TEnum.cc:64, 106` (cast checks):
  - Keep as-is
- [ ] `rewriter/Data.cc:51` (cast check):
  - Keep as-is
- [ ] `namer/namer.cc` (various single-level accesses):
  - Keep as-is
- [ ] `rewriter/ConstantAssumeType.cc:21, 35` (type checks):
  - Keep as-is
- [ ] `rewriter/Minitest.cc:41, 169, 242` (cast and cnst access):
  - Keep as-is
- [ ] `rewriter/ClassNew.cc:34, 57` (casts):
  - Keep as-is
- [ ] `rewriter/TypeMembers.cc:28` (cast):
  - Keep as-is

### Task 4.3: Document construction sites for Phase 6
- [ ] `rewriter/util/Util.cc:20, 27, 33` (make_unique):
  - Document for Phase 6 - no changes needed now
- [ ] `ast/TreeCopying.cc:158, 166` (make_expression):
  - Document for Phase 6 - no changes needed now
- [ ] Other construction sites:
  - All defer to Phase 6 when switching to vectorized form

### Task 4.4: Run tests
- [ ] Validate all changes with `bazel test //test:test_corpus`
- [ ] Ensure no behavioral changes

---

## Phase 6: Design Vectorized Representation

### Task 6.1: Design new internal structure
- [ ] Decide on data layout:
  ```cpp
  class UnresolvedConstantLit {
    core::LocOffsets loc;  // Location of first segment? Or entire range?
    std::vector<std::pair<core::NameRef, core::LocOffsets>> segments;
    ExpressionPtr rootScope;  // Root scope (EmptyTree, ConstantLit, etc.)
  };
  ```
  - **Key insight:** The `rootScope` field is the outermost non-UnresolvedConstantLit scope
  - For `ResolvedClass::A::B` where ResolvedClass is resolved:
    - `segments = [(A, locA), (B, locB)]`
    - `rootScope = ConstantLit(ResolvedClass)`
  - For `::A::B::C` (absolute path from root):
    - `segments = [(A, locA), (B, locB), (C, locC)]`
    - `rootScope = EmptyTree`
- [ ] Consider alternatives:
  - Separate vectors for names and locs?
  - Inline small vectors (e.g., `absl::InlinedVector<..., 3>`)?
  - Store total loc as range, individual locs as offsets?
- [ ] Decide on `loc` field semantics:
  - Keep as first segment's location for compatibility?
  - Change to span entire constant path?
  - **NEEDS INVESTIGATION:** Check what the current `loc` field represents

### Task 6.2: Estimate memory impact
- [ ] Calculate memory savings:
  - Current: 24 bytes × N segments + ExpressionPtr overhead
  - New: Base object + vector of pairs
- [ ] Consider cache locality improvements
- [ ] Validate against typical constant path patterns in Stripe codebase

### Task 6.3: Plan compatibility strategy
- [ ] Ensure iterators `parts()` and `partsReverse()` work unchanged
- [ ] Decide if `rootScope()` stays or becomes a direct field access
- [ ] Plan for any API changes needed

---

## Phase 7: Implement Vectorized Representation

### Task 7.1: Implement new structure
- [ ] Update `ast/Trees.h` with new data layout
- [ ] Update constructor in `ast/Trees.cc`
- [ ] Update `CheckSize` assertion (expect smaller size)

### Task 7.2: Update iterator implementations
- [ ] Reimplement `parts()` to return segments directly (or reversed)
- [ ] Reimplement `partsReverse()` to return segments directly (or reversed)
- [ ] Reimplement `rootScope()` to return root scope field

### Task 7.3: Update construction sites
- [ ] Update `Helpers.h::UnresolvedConstantParts` to build vectorized form:
  ```cpp
  // New signature (roughly):
  ExpressionPtr UnresolvedConstantParts(
    core::LocOffsets loc,
    ExpressionPtr rootScope,
    absl::Span<const std::pair<core::NameRef, core::LocOffsets>> parts
  );
  // Or keep existing signature and adapt internally
  ```
- [ ] Update all `make_expression<UnresolvedConstantLit>` call sites:
  - Change from single (name, loc) to vector of pairs
  - Add explicit rootScope parameter
  - All parts must be provided at construction time
- [ ] No builder pattern needed (all-at-once construction)

### Task 7.4: Update serialization/deserialization
- [ ] Check if UnresolvedConstantLit is serialized (e.g., for caching)
- [ ] Update serialization format if needed
- [ ] Ensure backward/forward compatibility if required

---

## Phase 8: Eliminate quadratic construction in `MK::UnresolvedConstant` ✅ COMPLETE

**Goal:** The `MK::UnresolvedConstant(loc, scope, name)` helper currently detects when
`scope` is an `UnresolvedConstantLit` and merges segments into a new flat UCL. This is
still quadratic: callers that chain `N` nested calls copy O(1)+O(2)+...+O(N) = O(N²)
segments. The fix is to push segment collection to the callers so every multi-segment UCL
is constructed in a single pass, then enforce that `UnresolvedConstant` is never called
with a UCL scope.

**Summary:**
- Added ENFORCE to `MK::UnresolvedConstant` that scope is never itself a UCL
- Fixed all callers: simple single-segment callers (arg reorder), multi-segment callers
  (flat construction), prism Translator chain-walking (new `buildConstantPath` helper),
  and Desugar.cc chain-walking (iterative walk over Const/ConstLhs nodes)
- Fixed Rails.cc which passed an existing UCL as scope (flatten by extracting parts)
- Fixed hello-test.cc which used the old 3-argument constructor
- All 2237 corpus tests pass + hello-test passes

### Task 8.1: Add ENFORCE to `MK::UnresolvedConstant` ✅
- [x] Remove the UCL-scope merging path in `ast/Helpers.h::UnresolvedConstant`
- [x] Add `ENFORCE(!cast_tree<UnresolvedConstantLit>(scope))` in its place
- [x] This will cause compilation failures at every caller that passes a UCL — those are
  the sites that need fixing in Task 8.2

### Task 8.2: Update callers to collect segments themselves ✅

#### Task 8.2.1: Simple single-segment rewriter callers (mechanical) ✅
These all call `UnresolvedConstant(loc, <non-UCL scope>, name)` — just reorder args
to `UnresolvedConstant(scope, {name}, {loc})`.

- [x] **`rewriter/TEnum.cc:151`**
  `MK::UnresolvedConstant(statLocZero, MK::EmptyTree(), name)`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {name}, {statLocZero})`

- [x] **`rewriter/Struct.cc:31`**
  `MK::UnresolvedConstant(loc, MK::EmptyTree(), Names::Constants::Elem())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::Elem()}, {loc})`

- [x] **`rewriter/Struct.cc:167`**
  `MK::UnresolvedConstant(loc, MK::Constant(loc, Symbols::root()), Names::Constants::Struct())`
  → `MK::UnresolvedConstant(MK::Constant(loc, Symbols::root()), {Names::Constants::Struct()}, {loc})`

- [x] **`rewriter/Concern.cc:107`**
  `MK::UnresolvedConstant(loc, MK::EmptyTree(), Names::Constants::ClassMethods())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::ClassMethods()}, {loc})`

- [x] **`rewriter/Concern.cc:137`**
  `MK::UnresolvedConstant(klass->loc, MK::EmptyTree(), Names::Constants::ClassMethods())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::ClassMethods()}, {klass->loc})`

- [x] **`rewriter/Data.cc:132`**
  `MK::UnresolvedConstant(loc, MK::Constant(loc, Symbols::root()), Names::Constants::Data())`
  → `MK::UnresolvedConstant(MK::Constant(loc, Symbols::root()), {Names::Constants::Data()}, {loc})`

- [x] **`rewriter/HasAttachedClass.cc:30`**
  `MK::UnresolvedConstant(zeroLoc, MK::EmptyTree(), Names::Constants::AttachedClass())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::AttachedClass()}, {zeroLoc})`

- [x] **`rewriter/Mattr.cc:121`**
  `MK::UnresolvedConstant(loc, MK::T(loc), Names::Constants::Boolean())`
  → `MK::UnresolvedConstant(MK::T(loc), {Names::Constants::Boolean()}, {loc})`

- [x] **`resolver/resolver.cc:1155`**
  `MK::UnresolvedConstant(loc, std::move(ancestor), enclosingClass.data(ctx)->name)`
  `ancestor` here is an existing ancestor expression (ConstantLit etc.), never a UCL.
  → `MK::UnresolvedConstant(std::move(ancestor), {enclosingClass.data(ctx)->name}, {loc})`

#### Task 8.2.2: Multi-segment rewriter callers using EmptyTree root (use UnresolvedConstantParts) ✅
These nest `UnresolvedConstant` calls with `EmptyTree` at the base — all segments share
the same `loc`, so `UnresolvedConstantParts` is the right replacement.

- [x] **`rewriter/Prop.cc:210`** (1 segment, Numeric)
  `MK::UnresolvedConstant(send->loc, MK::EmptyTree(), Names::Constants::Numeric())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::Numeric()}, {send->loc})`
  (single-segment, but listed here as it's in the middle of a multi-segment block)

- [x] **`rewriter/Prop.cc:222–232`** (4 segments: `Opus::Account::Model::Merchant`)
  Replace the 4-deep nesting with:
  `MK::UnresolvedConstantParts(send->loc, {Opus(), Account(), Model(), Merchant()})`

- [x] **`rewriter/Prop.cc:241–253`** (5 segments: `Opus::Autogen::Tokens::AccountModelMerchant::Token`)
  Replace the 5-deep nesting with:
  `MK::UnresolvedConstantParts(send->loc, {Opus(), Autogen(), Tokens(), AccountModelMerchant(), Token()})`

- [x] **`rewriter/Rails.cc:50–52`** (2 segments: `<recv>::Compatibility::<version>`)
  Outer scope is `send->recv` (not EmptyTree), so can't use `UnresolvedConstantParts`.
  Fixed by extracting existing UCL's parts via `cast_tree<UnresolvedConstantLit>(send->recv)`,
  iterating `recvUcl->parts()`, appending new segments, and building flat UCL from `recvUcl->rootScope_`.

#### Task 8.2.3: prism/Translator.cc single-segment cases (mechanical) ✅
These call `MK::UnresolvedConstant(loc, MK::EmptyTree(), name)` with a non-UCL scope.

- [x] **`parser/prism/Translator.cc:1127`**
  `MK::UnresolvedConstant(nameLoc, MK::EmptyTree(), name)`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {name}, {nameLoc})`

- [x] **`parser/prism/Translator.cc:1316`**
  `MK::UnresolvedConstant(nameLoc, MK::EmptyTree(), constantName)`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {constantName}, {nameLoc})`

- [x] **`parser/prism/Translator.cc:4629`**
  `MK::UnresolvedConstant(location, MK::EmptyTree(), Names::Constants::ConstantNameMissing())`
  → `MK::UnresolvedConstant(MK::EmptyTree(), {Names::Constants::ConstantNameMissing()}, {location})`

#### Task 8.2.4: prism/Translator.cc path cases (iterative chain walk) ✅
Added private `buildConstantPath(pm_node_t* parentNullable, LocOffsets outerLoc,
NameRef outerConstName, LocOffsets nullParentRootLoc)` helper to Translator that
iteratively walks `pm_constant_path_node` chains, collecting (NameRef, LocOffsets) pairs,
reversing, and constructing a flat UCL in one shot. Handles three cases: `PM_CONSTANT_PATH_NODE`
(walk deeper), `PM_CONSTANT_READ_NODE` (EmptyTree root), null parent (Constant root),
other nodes (desugar directly).

- [x] **`parser/prism/Translator.cc:1164`** — fixed via `buildConstantPath`
- [x] **`parser/prism/Translator.cc:1341`** — fixed via `buildConstantPath`
- [x] **`parser/prism/Translator.cc:4700`** — fixed: `translateConst` isConstantPath
  branch now calls `buildConstantPath` instead of `desugarNullable`→UCL-scope pattern

#### Task 8.2.5: ast/desugar/Desugar.cc (iterative chain walk) ✅
Both `parser::Const` and `parser::ConstLhs` handlers now walk BOTH types iteratively
using a `lastScopeRef` (pointer to `unique_ptr<Node>`) to retain ownership for dynamic
root scopes, collect (NameRef, LocOffsets) pairs, reverse, and call `MK::UnresolvedConstant`
once. Handles `nullptr` scope (EmptyTree root), `parser::Cbase` (Constant root with
`Symbols::root()`), and other nodes (call `node2TreeImpl` on the last scope ref).

- [x] **`ast/desugar/Desugar.cc:763–766`** (`parser::Const` handler)
- [x] **`ast/desugar/Desugar.cc:1561–1565`** (`parser::ConstLhs` handler — same pattern)

### Task 8.3: Fix compilation problems and test fallout ✅
- [x] Build with `bazel build //main:sorbet` after Task 8.1 to enumerate all broken callers
- [x] Work through each compilation error from Task 8.2 sites
- [x] Run `bazel test //test:test_corpus` to check for behavioral regressions — all 2237 pass
- [x] Run `bazel test //test:hello-test` — passes

---

## Risk Assessment & Considerations

### High-Risk Areas
1. **Scope field semantics:** The `scope` field can be different types (UnresolvedConstantLit, EmptyTree, ConstantLit). Need to handle all cases in iteration.
2. **Location tracking:** Each segment has its own location. Must preserve for error reporting.
3. **Construction patterns:** Code that builds constants bottom-up will need significant changes.
4. **Tree walking/mutation:** Treemap and mutation passes may have assumptions about structure.

### Open Questions

1. **DECIDED: Iterator yields (NameRef, LocOffsets) pairs**
   - Each iterator step provides access to both the name and location of that segment

2. **DECIDED: Use C++ iterator concepts with operator++ only**
   - Proper iterator classes implementing ForwardIterator
   - No need for random access or operator--

3. **DECIDED: Construction API design - All-at-once (Option A)**
   - Constructor will require complete vector of parts at construction time
   - No incremental building or builder pattern needed
   - Aligns with existing helper pattern: `UnresolvedConstantParts(loc, {nameA, nameB, nameC})`
   - Simpler implementation, more efficient (no intermediate allocations)

4. **DECIDED: Partially resolved constants (scope is ConstantLit)**
   - Iterator yields only the UnresolvedConstantLit names/locs up to the ConstantLit scope
   - The outermost non-UnresolvedConstantLit scope (EmptyTree, ConstantLit, etc.) is accessible via `rootScope()` helper
   - Example: For `ResolvedClass::A::B` where ResolvedClass is a ConstantLit:
     - Iterator yields: `[A, B]`
     - `rootScope()` returns: the ConstantLit for ResolvedClass

5. **Should we keep the outer `loc` field?**
   - Current: Points to first segment's location (or the location of the entire expression?)
   - Could change to span entire range (first segment start to last segment end)
   - **NEEDS INVESTIGATION:** What does the current `loc` field actually represent?

6. **Performance trade-offs:**
   - Vectorized form saves memory but makes appending harder
   - Is append performance important? (Probably not during normal operation)

---

## Usage Sites to Migrate

### Summary Statistics
- **Total cast_tree uses:** 43 (excluding ast/Trees.* and test/)
- **Files with usage:** 15 unique files
- **Most complex patterns:** packager/packager.cc, namer/namer.cc, resolver/resolver.cc

### Migration Phase Breakdown
- **Phase 3 (Mutable Iterators):** Add support for mutating fields through iteration
  - Implement `MutableReverseRange` with `MutableSegment` struct
  - Required for 3 mutation sites that modify `cnst` or `scope` fields
- **Phase 4 (Recursive Traversal):** 10 sites that walk the linked list
  - Will be converted to use `parts()`, `partsReverse()`, or `partsMutable()` iterators
  - Includes: resolver, packager, namer, rewriter/util, local_vars, desugar, substitute, struct
- **Phase 5 (Non-Traversal):** 30+ sites with simpler access patterns
  - Type checking sites: Use `rootScope()` helper (6 sites)
  - Single-level access: Keep as-is (20+ sites)
  - Construction: Document for Phase 7 (10+ sites)

---

## PHASE 3 SITES: Recursive Traversal (10 locations)

These sites walk the linked list structure and will be migrated to use iterators.

### 1. RECURSIVE TRAVERSAL SITES

#### packager/packager.cc:51-62
- **Pattern:** Loop building `fullNameReversed` vector from entire chain
- **Fields:** `->cnst`, `->scope`
- **Migration:** Use `partsReverse()` iterator
- **Notes:** Handles transition to ConstantLit at end

#### packager/packager.cc:1109-1123
- **Pattern:** Loop testing each level for `Test()` constant
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `partsReverse()` iterator with early break
- **Notes:** Checks `isa_tree<EmptyTree>(scope)` at each level

#### resolver/resolver.cc:1156-1179
- **Pattern:** Recursive `walkUnresolvedConstantLit` converting to ConstantLit
- **Fields:** `->scope`
- **Migration:** Use `parts()` or `partsReverse()` iterator
- **Notes:** This is the main resolution pass

#### rewriter/util/Util.cc:395-410
- **Pattern:** `isRootScopedSyntacticConstant` validation loop
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `parts()` iterator and compare against expected sequence
- **Notes:** Reverse iteration from end checking each name

#### rewriter/PackageSpec.cc:28-52
- **Pattern:** Package name validation (no underscores)
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `parts()` or `partsReverse()` iterator
- **Notes:** Loop checking `shortName` contains `_`

#### local_vars/local_vars.cc:496-506
- **Pattern:** Recursive `walkConstantLit`
- **Fields:** `->scope`
- **Migration:** Use iterator or keep recursive with `rootScope()`
- **Notes:** May not need iterator if just processing scope once

#### ast/desugar/Desugar.cc:2315-2325
- **Pattern:** Building `args` vector from constant chain for `defined?`
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `partsReverse()` iterator (needs reverse order)
- **Notes:** Creates MK::String for each segment

#### rewriter/Struct.cc:35-48
- **Pattern:** Recursive `selfScopeToEmptyTree` mutation
- **Fields:** `->scope`
- **Migration:** May need special handling for mutation
- **Notes:** Modifies scope in-place, tricky for vectorized form

#### ast/substitute/Substitute.cc:14-28
- **Pattern:** `substClassName` loop mutating `cnst` at each level
- **Fields:** `->cnst`, `->scope`
- **Migration:** May need special handling for mutation
- **Notes:** Modifies `cnst` in-place, tricky for vectorized form

#### ast/packager/packager.cc:8-16
- **Pattern:** Find last constant to prepend registry
- **Fields:** `->scope`
- **Migration:** Use `rootScope()` and modify that
- **Notes:** Walks to end then mutates last scope

#### namer/namer.cc:142-160
- **Pattern:** Recursive `defineScope` building hierarchy
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `parts()` iterator
- **Notes:** Builds FoundClass hierarchy from chain

---

## PHASE 4 SITES: Non-Traversal (30+ locations)

These sites use simpler access patterns and mostly won't need changes.

---

### 3. SINGLE-LEVEL ACCESS SITES (20+ locations)

#### rewriter/util/Util.cc:12-34 (dupUnresolvedConstantLit)
- **Pattern:** Duplicate node with duplicated scope
- **Fields:** `->loc`, `->scope`, `->cnst`
- **Migration:** Keep as-is, accesses immediate fields only
- **Notes:** Lines 20, 27, 33 - construction with same structure

#### rewriter/util/Util.cc:17-22
- **Pattern:** Check immediate scope type (EmptyTree vs ConstantLit)
- **Fields:** `->scope`
- **Migration:** Could use `rootScope()` helper
- **Notes:** Simple type checking

#### rewriter/Concern.cc:25-40
- **Pattern:** Check 2-level pattern (ActiveSupport::Concern)
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use iterator or manual access of first 2 levels
- **Notes:** Specific 2-level validation

#### ast/TreeCopying.cc:156-167
- **Pattern:** Deep copy with deepCopy of scope
- **Fields:** `->loc`, `->scope`, `->cnst`
- **Migration:** Keep as-is, construction pattern
- **Notes:** Lines 158, 166

#### rewriter/Struct.cc:35-48
- **Pattern:** Check scope type and recurse
- **Fields:** `->scope`
- **Migration:** Use `rootScope()` or iterator
- **Notes:** Type checking EmptyTree, ConstantLit, self

#### rewriter/Prop.cc:24-39
- **Pattern:** Check if `struct_->cnst` matches specific name
- **Fields:** `->cnst`, `->scope`
- **Migration:** Keep as-is, single-level check
- **Notes:** Lines 25, 31, 37 - check cnst == Struct/TStruct/ImmutableStruct

#### rewriter/TEnum.cc:64, 106
- **Pattern:** Simple cast checks
- **Fields:** None directly
- **Migration:** Keep as-is
- **Notes:** Just checking if cast succeeds

#### rewriter/Data.cc:51
- **Pattern:** Simple cast check
- **Fields:** None
- **Migration:** Keep as-is
- **Notes:** Just casting lhs

#### local_vars/local_vars.cc:497-505
- **Pattern:** Type check and recursive call
- **Fields:** `->scope`
- **Migration:** Keep as-is or use `rootScope()`
- **Notes:** Simple type checks

#### namer/namer.cc:185-187, 306-307, 525-532
- **Pattern:** Various single-level accesses
- **Fields:** `->cnst`, `->scope`
- **Migration:** Keep as-is
- **Notes:** Direct field access for immediate values

#### rewriter/ConstantAssumeType.cc:21, 35
- **Pattern:** Type checking with isa_tree
- **Fields:** None
- **Migration:** Keep as-is
- **Notes:** Just type checks

#### rewriter/Minitest.cc:41, 169, 242
- **Pattern:** Cast checks and cnst access
- **Fields:** `->cnst`, `->loc`
- **Migration:** Keep as-is
- **Notes:** Single-level access only

#### rewriter/ClassNew.cc:34, 57
- **Pattern:** Simple casts
- **Fields:** None
- **Migration:** Keep as-is
- **Notes:** Type checking only

#### rewriter/TypeMembers.cc:28
- **Pattern:** Simple cast
- **Fields:** None
- **Migration:** Keep as-is
- **Notes:** Type checking only

---

### 4. TYPE CHECKING SITES (6 locations)

#### rewriter/util/Util.cc:17-27
- **Pattern:** Check if scope is EmptyTree vs UnresolvedConstantLit vs ConstantLit
- **Fields:** `->scope`
- **Migration:** Could use `rootScope()` and type check that
- **Notes:** Line 19 - isa_tree checks

#### local_vars/local_vars.cc:497-506
- **Pattern:** Type checks on scope
- **Fields:** `->scope`
- **Migration:** Use `rootScope()` helper
- **Notes:** Lines 499-500 - isa_tree checks

#### rewriter/PackageSpec.cc:80-96
- **Pattern:** Check if name is UnresolvedConstantLit
- **Fields:** None (uses isa_tree)
- **Migration:** Keep as-is
- **Notes:** Type checking only

#### packager/packager.cc:1111-1115
- **Pattern:** Check if scope is EmptyTree
- **Fields:** `->scope`
- **Migration:** Use `rootScope()` helper
- **Notes:** Line 1111 - isa_tree check

#### rewriter/Struct.cc:35-48
- **Pattern:** Multiple type checks on scope
- **Fields:** `->scope`
- **Migration:** Use `rootScope()` helper
- **Notes:** Lines 36-40 - various type checks

#### ast/Helpers.h:623-624
- **Pattern:** Check if root scope (already documented)
- **Fields:** `->scope`, `->cnst`
- **Migration:** Use `rootScope()` helper
- **Notes:** Check cnst and isRootScope(scope)

---

### 5. CONSTRUCTION SITES (10+ locations)

#### rewriter/util/Util.cc:20, 27, 33
- **Pattern:** make_unique<UnresolvedConstantLit>
- **Fields:** Uses `->loc`, `->cnst` from source
- **Migration:** No change needed for now (still linked list)
- **Notes:** Will need change in Phase 5

#### ast/TreeCopying.cc:158, 166
- **Pattern:** make_expression<UnresolvedConstantLit>
- **Fields:** `->loc`, `->scope`, `->cnst`
- **Migration:** No change needed for now
- **Notes:** Deep copy pattern

#### rewriter/PackageSpec.cc:120-121, 125
- **Pattern:** Convert to ConstantLit or call prependRegistry
- **Fields:** Various
- **Migration:** No change needed
- **Notes:** Conversion patterns

#### namer/namer.cc:1947, 2068, 2242
- **Pattern:** Convert to ConstantLit with toUnique
- **Fields:** Various
- **Migration:** No change needed
- **Notes:** Resolution patterns

---

### 6. SPECIAL/SERIALIZATION SITES (1 location)

#### core/serialize/serialize.cc:1266-1269
- **Pattern:** Pickle serialization
- **Fields:** Entire object
- **Migration:** Will need to update serialization format in Phase 5
- **Notes:** Critical - must handle backward compatibility

---

## High-Risk Migration Sites

### Mutation Sites (CRITICAL)
1. **ast/substitute/Substitute.cc:14-28** - Mutates `cnst` field in loop
2. **rewriter/Struct.cc:35-48** - Mutates `scope` field recursively
3. **ast/packager/packager.cc:8-16** - Mutates last `scope` in chain

### Complex Traversal Sites
1. **packager/packager.cc:51-62** - Complex loop with ConstantLit transition
2. **resolver/resolver.cc:1156-1179** - Main resolution pass
3. **namer/namer.cc:142-160** - Recursive scope definition building

### Serialization Sites
1. **core/serialize/serialize.cc:1266-1269** - Pickle format must be updated

---

## Timeline Estimate

**DO NOT TAKE AS COMMITMENT - for planning purposes only**

- Phase 1 (Iterators): Small, self-contained ✅ COMPLETE
- Phase 2 (Audit): Medium, requires thorough code search ✅ COMPLETE
- Phase 3 (Mutable Iterators): Small, add MutableReverseRange
- Phase 4 (Recursive Traversal Migration): Medium-Large, 10 sites to migrate
- Phase 5 (Non-Traversal Sites): Small-Medium, mostly using rootScope() helper
- Phase 6 (Design Vectorized): Small, mostly decision-making
- Phase 7 (Implement Vectorized): Medium, structural changes
- Phase 8 (Testing): Medium, validation and fixes

Total effort: Multiple days to weeks depending on complexity of migration.

---

## Success Criteria

1. ✅ All existing tests pass
2. ✅ Memory usage reduced by >50% for typical constant paths
3. ✅ No performance regressions in parse/resolve passes
4. ✅ Code is cleaner and more maintainable
5. ✅ Iterator abstraction makes future changes easier

---

## Task 9: Restructure `walkUnresolvedConstantLit` to Handle Flat Multi-Segment UCLs Natively

**Goal:** Currently, `walkUnresolvedConstantLit` in `resolver/resolver.cc` explodes a flat
`UnresolvedConstantLit` (UCL) with N segments into a chain of N single-segment
`ConstantLit`-wrapping UCLs. This undoes the memory savings from the flat vectorized UCL
representation introduced by earlier phases. This task restructures the resolver so that a
flat N-segment UCL is wrapped in exactly ONE `ConstantLit`, and the constant resolution
machinery is updated to resolve multi-segment UCLs correctly through incremental progress
tracking.

**Invariant to maintain:** After resolution, each `ast::ConstantLit` that wraps an
`ast::UnresolvedConstantLit` contains the resolved symbol for the ENTIRE qualified path
(the leaf/final segment's resolved symbol).

**Current behavior (wrong):** For `A::B::C` (a flat UCL with `segments_=[A,B,C]`,
`scope_=EmptyTree`), `walkUnresolvedConstantLit` creates three `ConstantLit`s:
```
ConstantLit(C_sym, UCL{[C], scope=ConstantLit(B_sym, UCL{[B], scope=ConstantLit(A_sym, UCL{[A], scope=EmptyTree})})})
```
This is O(N) allocations per UCL and undoes the flat representation.

**Target behavior:** Create ONE `ConstantLit(C_sym, UCL{[A,B,C], scope=EmptyTree})`.

---

### Subtask 9.1: Add progress-tracking fields to `ConstantResolutionItem`

**File:** `resolver/resolver.cc`

Currently:
```cpp
struct ConstantResolutionItem {
    shared_ptr<Nesting> scope;
    ast::ConstantLit *out;
    bool resolutionFailed = false;
};
```

Add two fields for incremental multi-segment resolution:
```cpp
// How many segments of out->original()->segments_ have been resolved.
// -1 = none resolved yet.
// When resolvedUpToSegment == (segCount - 1), all segments are done.
int resolvedUpToSegment = -1;

// The DEALIASED ClassOrModuleRef of the segment at index resolvedUpToSegment.
// Used as the starting scope for the next segment lookup on retry.
// Meaningful only when resolvedUpToSegment >= 0.
core::ClassOrModuleRef resolvedOwner;
```

These fields let `resolveConstant` skip re-resolving already-resolved prefixes on retry,
preserving fixed-point convergence without quadratic re-work.

---

### Subtask 9.2: Restructure `walkUnresolvedConstantLit` to create ONE `ConstantLit` per flat UCL

**File:** `resolver/resolver.cc` (~line 1169)

Replace the current loop-over-segments (which creates N `ConstantLit`s) with a single
`ConstantLit` wrapping the whole flat UCL:

```cpp
void walkUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr &tree) {
    if (auto c = ast::cast_tree<ast::UnresolvedConstantLit>(tree)) {
        // Process root scope first (invariant: scope_ is never a UCL)
        walkUnresolvedConstantLit(ctx, c->scope_);

        // Create ONE ConstantLit for the entire flat UCL
        auto out = ast::make_expression<ast::ConstantLit>(
            core::Symbols::noSymbol(), tree.toUnique<ast::UnresolvedConstantLit>());
        auto constant = ast::cast_tree<ast::ConstantLit>(out);
        ConstantResolutionItem job{nesting_, constant};
        if (resolveConstantJob(ctx, job)) {
            categoryCounterInc("resolve.constants.nonancestor", "firstpass");
            if (this->loadTimeScope()) {
                // checkReferenceOrder for each resolved segment (see 9.6)
            }
        } else {
            todo_.emplace_back(std::move(job));
        }
        tree = std::move(out);
        return;
    }
    // ... unchanged: EmptyTree, ConstantLit, and dynamic cases
}
```

---

### Subtask 9.3: Restructure `resolveConstant` for multi-segment UCLs

**File:** `resolver/resolver.cc` (~line 362)

The core logic change: instead of checking a single `c.cnst()` against either an EmptyTree
or ConstantLit scope, iterate through all segments of the flat UCL, resuming from
`job.resolvedUpToSegment` when retrying.

Key design decisions:
- **EmptyTree scope:** Look up `segments_[0]` via `resolveLhs(ctx, job.scope, name)` (nesting-aware
  lookup, same as today for the first segment). If that resolves, dealias and use as owner for
  `segments_[1]`, etc.
- **ConstantLit scope:** (For `::A::B` form.) `scope_` ConstantLit must already be resolved
  (guaranteed since `walkUnresolvedConstantLit` processes the scope first). Dealias it and use as
  owner for `segments_[0]`, then `segments_[1]`, etc.
- **Dynamic scope:** (E.g., a Send.) Mark `resolutionFailed = true` and error, same as today.
- **Progress tracking:** When we successfully resolve `segments_[i]` but `segments_[i+1]` is not
  yet resolvable, set `job.resolvedUpToSegment = i` and `job.resolvedOwner = dealiased_sym_i`
  (a `ClassOrModuleRef`), then return `noSymbol`. On retry, skip straight to segment `i+1`.
- **Return value:** Always return the pre-dealias symbol of the LAST (leaf) segment, consistent
  with current behavior for single-segment UCLs.
- **Single-segment fast path:** When `segCount == 1` and scope is EmptyTree, behavior is identical
  to today's code: `return resolveLhs(ctx, job.scope, segments_[0].name)`.

**Type-alias check:** For segments `1..n-1` (explicitly scoped), check if the previously
resolved segment was a type alias. If so, error "Resolving constants through type aliases is
not supported" and set `resolutionFailed = true`. (Same semantics as the existing ConstantLit
scope case.)

**Private constant check:** The existing private-constant check in the ConstantLit scope path
applies for segments `1..n` (those after the nesting-resolved first segment). For segment 0 with
EmptyTree scope, no private check (same as today).

---

### Subtask 9.4: Minimal updates to `resolveConstantJob`

**File:** `resolver/resolver.cc` (~line 561)

`resolveConstantJob` calls `resolveConstant` and calls `job.out->setSymbol(resolved)`. Since
`resolveConstant` now mutates `job.resolvedUpToSegment` and `job.resolvedOwner` in-place as it
makes progress, `resolveConstantJob` needs no structural changes. The `isAlreadyResolved`
fast-path is also unchanged.

Verify that the `isTypeAlias` path (returning `false` until the type alias's `resultType` is set)
still works correctly with multi-segment UCLs.

---

### Subtask 9.5: Update `constantResolutionFailed` for multi-segment UCLs

**File:** `resolver/resolver.cc` (~line 432)

Currently, error messages reference `original.cnst()` (last segment name) and `original.loc`
(last segment loc). For multi-segment UCLs, the error should target the FIRST UNRESOLVED segment.

Changes needed:
- Determine `failedSegIdx = job.resolvedUpToSegment + 1` (0 if nothing resolved).
- Use `c.segments_[failedSegIdx].first` as the failing name for the error message.
- Use `c.segments_[failedSegIdx].second` as the failing loc for error location.
- The `constantNameMissing` check should use `c.segments_[failedSegIdx].first` (not `c.cnst()`).
- The "did you mean" fuzzy match should use `c.segments_[failedSegIdx].first`.

**`resolutionScopes` population:**
- If `job.resolvedUpToSegment >= 0` (some segments resolved): the scope is the resolved owner
  symbol. Insert `job.resolvedOwner` as the single explicit scope (analogous to the ConstantLit
  scope path today).
- If `job.resolvedUpToSegment == -1` and `scope_` is ConstantLit: use `id->symbol().dealias(ctx)`
  as the scope (same as today for single-segment explicit scope).
- If `job.resolvedUpToSegment == -1` and `scope_` is EmptyTree: fill in the full nesting chain
  (same as today for nesting scope).

**StubModule propagation:** If a previously resolved segment's owner dealiases to StubModule, set
`alreadyReported = true` to suppress the cascade error (same semantics as today's
"`C` was already stubbed" check).

---

### Subtask 9.6: Update `checkReferenceOrder` and its call sites

**File:** `resolver/resolver.cc` (~line 331)

`checkReferenceOrder` currently takes `const ast::UnresolvedConstantLit &c` and uses `c.loc`
(leaf loc) as the reference location. For multi-segment UCLs, each segment needs its own call.

**Change the signature** to take a `core::LocOffsets` instead of the whole UCL:
```cpp
static void checkReferenceOrder(core::Context ctx, core::SymbolRef resolutionResult,
                                core::LocOffsets refLoc,
                                const UnorderedMap<core::SymbolRef, core::LocOffsets> &firstDefinitionLocs);
```

**In `walkUnresolvedConstantLit`** (after first-pass success): walk the resolved segments to call
`checkReferenceOrder` once per segment. To get each segment's resolved symbol, `resolveConstant`
can be extended (or a separate helper written) to report per-segment symbols. The simplest approach:
after `resolveConstantJob` succeeds, walk `c.segments_` root-to-leaf, re-resolving each one (using
the stored `resolvedOwner` to start from the already-dealiased ancestor).

**In the retry loop:** When a previously deferred item finally resolves, also call
`checkReferenceOrder` for each segment. This requires re-running the per-segment walk after
successful resolution in `resolveConstantResolutionItems`.

Alternatively (simpler), only check the leaf segment's reference order (conservative: any
out-of-order error for `A::B::C` is reported at `C`'s loc). This is a demotion of diagnostic
quality but not correctness. Mark as a known simplification if taken.

---

### Subtask 9.7: Update `constantDepth` for flat UCLs

**File:** `resolver/resolver.cc` (~line 1536)

Currently `constantDepth` counts the depth of a ConstantLit chain by walking
`original()->scope_` ConstantLit pointers. With flat UCLs, there is no chain, so this returns
0 for all constants. The sort (parents before children) becomes loc-only, which should be
correct (outer names appear at earlier locs) but loses some determinism when two constants
share a loc.

Update to use segment count as the depth proxy:
```cpp
static int constantDepth(ast::ConstantLit *exp) {
    int depth = 0;
    if (auto original = exp->original()) {
        depth += (int)original->segments_.size() - 1;
        // Walk any ConstantLit scope (e.g. ::A::B absolute reference)
        auto scopeLit = ast::cast_tree<ast::ConstantLit>(original->scope_);
        while (scopeLit && scopeLit->original()) {
            depth += (int)scopeLit->original()->segments_.size();
            scopeLit = ast::cast_tree<ast::ConstantLit>(scopeLit->original()->scope_);
        }
    }
    return depth;
}
```

This ensures `A` (depth 0) sorts before `A::B` (depth 1) sorts before `A::B::C` (depth 2).

---

### Subtask 9.8: Verify `transformAncestor` ENFORCE

**File:** `resolver/resolver.cc` (~line 1145)

The existing ENFORCE:
```cpp
ENFORCE(sym.exists() || ast::isa_tree<ast::ConstantLit>(cnst->original()->scope_) ||
        ast::isa_tree<ast::EmptyTree>(cnst->original()->scope_));
```
With flat UCLs, `cnst->original()->scope_` is always EmptyTree or ConstantLit(root/other resolved
scope). This ENFORCE should hold as-is. Verify after the other changes by running the test suite.

---

### Subtask 9.9: Verify `autogen.cc` handles flat multi-segment UCLs

**File:** `main/autogen/autogen.cc`

The autogen pass walks `ConstantLit` nodes and inspects `original()->scope_` and the UCL's
segments. After this task's changes, each `ConstantLit` in the resolver output will wrap a flat
multi-segment UCL. The autogen code was already updated to iterate `segments_` rather than
walking a linked list, so it should handle this correctly. Run autogen-specific tests to confirm.

---

### Subtask 9.10: Verify `packager/packager.cc` handles flat multi-segment UCLs

**File:** `packager/packager.cc`

The packager walks `ConstantLit` nodes (and sometimes the UCLs they wrap). After this change,
the structure of resolved `ConstantLit` nodes changes from chains to flat-wrapping. Verify that
packager logic (name extraction, test-constant detection, etc.) still works. Run packager-specific
tests.

---

### Subtask 9.11: Handle `ConstantLit::resolutionScopes` and `markUnresolved`

No structural change needed: `markUnresolved` still allocates the `resolutionScopes` vector on
the single `ConstantLit`. The change from subtask 9.5 ensures this vector is populated with the
correct scopes (from the failing segment's context, not the leaf segment's context).

Verify: after resolution fails, `ConstantLit::resolutionScopes()` contains the scopes that were
searched for the FIRST unresolved segment, consistent with how LSP uses it in `DefLocSaver.cc`.

---

### Subtask 9.12: Testing and validation

- `bazel test //test:test_corpus` — all ~6600+ tests must pass
- `bazel test //test:test_LSPTests` — all LSP tests must pass
- `bazel test //test:hello-test` — must pass
- Pay special attention to tests involving:
  - Multi-segment constants: `A::B::C`, `::A::B::C`
  - Constants used as ancestors (superclass, include, extend)
  - Unresolved constant error messages and locations
  - Out-of-order constant access checks
  - Constants with explicit scope that fail to resolve
  - StubModule / cascade-error suppression
  - Type alias constant resolution errors
