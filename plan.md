# Design: Merge `getSelection`, `isInStatementContext`, `isReturnValueNeeded`

## Problem

`getExtractMethodEdits` currently performs three separate top-down AST traversals to answer
three related questions about the same selection:

1. **`getSelection`** — find the selection (sequence of nodes whose locs are contained in the target)
2. **`isInStatementContext`** — is the selection in a statement position?
3. **`isReturnValueNeeded`** — is the return value of the selection used?

All three functions share identical structure: the same early-exit conditions, the same
`InsSeq` / `While` / `Assign` / default switch cases, and the same stop-on-first-match
recursion pattern. The two boolean functions exist only to populate `SelectionContext`, which
`getSelection` already accepts as a parameter (recently added). The goal is a single traversal
that produces a `SelectionResult` carrying both the selected nodes and the correct
`SelectionContext`.

---

## Current state

### `SelectionContext`

```cpp
struct SelectionContext {
    bool isInStat = false;
    bool isRetValueNeeded = false;
};
```

Holds the two booleans that are currently computed by the two separate functions.

### `SelectionResult`

```cpp
class SelectionResult {
    SelectionResult(vector<const ast::ExpressionPtr *> stats, SelectionContext selCx)
        : stats_{stats}, selCx_{selCx} {
        ENFORCE(stats_.empty()); // BUG: should be ENFORCE(!stats_.empty())
    }
    ...
};
```

Already pairs the selection with a context. Has a bug in the ENFORCE (see below).

### `getSelection` (lines 419–478)

Takes `SelectionContext selCx` but does not yet propagate it correctly through the
recursion — internal recursive calls omit the third argument (won't compile), and the
`InsSeq` multi-stat path returns a raw `vector<>` instead of a `SelectionResult`.

### `isInStatementContext` (lines 922–976)

Recurses with a `bool inStat` parameter. Rule table:

| Location              | `inStat` passed to child |
|-----------------------|--------------------------|
| InsSeq stats/expr     | `true`                   |
| While cond            | `false`                  |
| While body            | `true`                   |
| All other children    | `false`                  |
| Multi-stat InsSeq hit | returns `true`           |

Top-level call: `isInStatementContext(tree, offsets, /*inStat=*/true)`.

### `isReturnValueNeeded` (lines 978–1034)

Recurses with a `bool isNeeded` parameter. Rule table:

| Location                          | `isNeeded` passed to child              |
|-----------------------------------|-----------------------------------------|
| InsSeq — each stat                | `stat == stats.back()` (last expr only) |
| While cond                        | `true`                                  |
| While body                        | `false`                                 |
| All other children                | `true`                                  |
| Multi-stat InsSeq hit             | `j == stats.size()` (last expr included?)|

Top-level call: `isReturnValueNeeded(tree, offsets, /*isNeeded=*/true)`.

---

## Proposed changes

### 1. Fix the `ENFORCE` bug in `SelectionResult`

```cpp
// Before
ENFORCE(stats_.empty());
// After
ENFORCE(!stats_.empty());
```

### 2. Merge `isInStatementContext` and `isReturnValueNeeded` into `getSelection`

Propagate a `SelectionContext` top-down through `getSelection`, setting both fields
simultaneously on the way down, matching the existing logic of the two separate functions.

The merged signature is unchanged: `SelectionResult getSelection(const ast::ExpressionPtr &expr, const core::LocOffsets target, SelectionContext selCx)`.

#### How `selCx` is built for each recursive call

**InsSeq — single-child recursion:**
```
for each stat in (insSeq.stats ++ [insSeq.expr]):
    child selCx = {
        isInStat        = true,
        isRetValueNeeded = (stat == stats.back()),
    }
```

**InsSeq — multi-stat range hit:**
```
selCx for the returned SelectionResult = {
    isInStat        = true,
    isRetValueNeeded = (j == stats.size()),   // selection reaches last expr?
}
```

**While:**
```
cond:  selCx = { isInStat = false, isRetValueNeeded = true  }
body:  selCx = { isInStat = true,  isRetValueNeeded = false }
```

**Assign (rhs only — lhs is never a valid selection):**
```
rhs:   selCx = { isInStat = false, isRetValueNeeded = true  }
```
This matches what the `default` branch would compute for `isInStatementContext` (passes
`false` to children) and `isReturnValueNeeded` (passes `true` to children). The special
`Assign` case in `getSelection` already limits recursion to `rhs`, which we keep — the lhs
of an assignment is a write target, not an extractable expression.

**Default (all other parents):**
```
each child:  selCx = { isInStat = false, isRetValueNeeded = true }
```

**Base case — `target.contains(expr.loc())`:**
```
return SelectionResult({&expr}, selCx);   // selCx from caller is the answer
```

**Top-level call site in `getExtractMethodEdits`:**
```
Initial selCx = { isInStat = true, isRetValueNeeded = true }
```
This mirrors the two existing top-level calls.

### 3. Fix the `InsSeq` multi-stat return path

Currently the multi-stat path builds a `vector<>` and returns it directly. After the merge:
```cpp
auto [i, j] = result.value();
bool includesLastExpr = (j == (int)stats.size());
SelectionContext multiStatCx{true, includesLastExpr};
vector<const ast::ExpressionPtr *> selection;
for (int k = i; k < j; k++) {
    selection.push_back(stats[k]);
}
return {selection, multiStatCx};
```

### 4. Update `getExtractMethodEdits`

```cpp
// Before (3 separate traversals)
auto selection        = getSelection(parsedFile.tree, selectionLoc.offsets());
auto isInStat         = isInStatementContext(parsedFile.tree, selectionLoc.offsets(), true);
auto isRetValueNeeded = isReturnValueNeeded(parsedFile.tree, selectionLoc.offsets(), true);

// After (single traversal)
SelectionContext initialCx{true, true};
auto selResult = getSelection(parsedFile.tree, selectionLoc.offsets(), initialCx);
if (!selResult.has_value()) { ... }
auto selCx = selResult.selectionContext();
// selCx.isInStat        replaces isInStat.value()
// selCx.isRetValueNeeded replaces isRetValueNeeded.value()
// selResult.stats()      replaces selection
```

The two `ENFORCE(isInStat.has_value())` / `ENFORCE(isRetValueNeeded.has_value())` checks
are already implied by the single `!selResult.has_value()` guard — `SelectionContext` is
always set when the selection is found.

### 5. Delete `isInStatementContext` and `isReturnValueNeeded`

Both functions are deleted entirely after their logic is folded into `getSelection`.

---

## Correctness argument

The three functions agree on tree structure: same early-exit conditions, same recursive
descent. The only difference is the value threaded through. By combining the threaded values
into `SelectionContext` and passing them together, we compute exactly the same answer in one
pass.

The only non-trivial case is the **InsSeq multi-stat hit**: `isInStatementContext` ignores
the outer `selCx.isInStat` and always returns `true`; `isReturnValueNeeded` ignores the
outer `selCx.isRetValueNeeded` and computes from the range bounds. The merged code does the
same: it constructs a fresh `multiStatCx` and ignores the incoming selCx for that path.

---

## Risks and edge cases

- **`Assign` lhs**: the existing `getSelection` intentionally skips `assign.lhs`. The
  merged function preserves this. If a user selects exactly `assign.lhs`, `getSelection`
  returns empty — correct, since that is not a valid extraction target.

- **`InsSeq.expr` is the last stat**: `stats.back()` compares pointer equality after
  `stats.push_back(&insSeq.expr)`, so `isRetValueNeeded = true` correctly fires for the
  final expression.

- **Empty / non-existent locs**: unchanged early exits at the top of `getSelection` ensure
  these are handled before any selCx logic is consulted.

- **`getSelection` called with only 2 args internally (current bug)**: all internal
  recursive calls must be updated to pass the appropriate `selCx`. This is required for the
  code to compile and is the main mechanical change.

- **Existing test coverage**: `test/run_lsp_tests.py` should be run after the change. No
  new tests are needed — behavior is identical.

---

## Summary of changes

| What                                  | Action              |
|---------------------------------------|---------------------|
| `ENFORCE(stats_.empty())`             | Fix to `!stats_.empty()` |
| `getSelection` — internal 2-arg calls | Add correct `selCx` arg to each |
| `getSelection` — InsSeq multi-stat    | Return `SelectionResult` with computed `multiStatCx` |
| `getSelection` — all recursive calls  | Pass the correct `selCx` per the rule table above |
| `isInStatementContext`                | Delete |
| `isReturnValueNeeded`                 | Delete |
| `getExtractMethodEdits` call site     | Single `getSelection` call; read context from `selResult.selectionContext()` |
