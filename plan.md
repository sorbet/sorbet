# Plan: Add `ast::Tag::Send` case to `getContinuation`

## Context

`getContinuation` (in [main/lsp/ExtractMethod.cc](main/lsp/ExtractMethod.cc)) walks the
tree and, for a selection contained in some node, builds an approximation of the code that
can run *after* the selection. This continuation feeds the liveness analysis that decides
which variables the extracted method must return.

Today `Send` falls into the `default` case (lines 650-663), which iterates over children
in evaluation order; once the child containing the selection is found, every subsequent
child is appended to the continuation as an ordinary statement. A `Send`'s children (per
`iterChildrenUntil`, lines 221-234) are, in order: `recv`, `nonBlockArgs()`, and the
`rawBlock()`.

The problem: a block passed to a `Send` (e.g. `xs.each { ... }`) is a **loop body** — it
may run zero or more times. When the selection is *inside* the block body, the code after
the selection can loop back and re-run the block body. The `default` case doesn't model
this, so liveness is under-approximated for selections inside blocks. This mirrors the
existing `While` case (lines 602-615), where selecting inside `while_.body` appends the
loop-back branch `Branch(empty, body)`.

## Change

Add an explicit `case ast::Tag::Send` to the `switch` in `getContinuation`, placed
alongside the other structured cases (e.g. before the `default` case). It behaves like the
`default` case for `recv` and the non-block args, but treats the block as a loop body:

1. Walk `recv`, then each `nonBlockArgs()` element, exactly like the `default` case: if the
   continuation is already found, append the child as a statement; otherwise try to find
   the continuation inside the child (via a `handleChild` helper).

2. If `rawBlock()` is present, handle the block explicitly rather than recursing on the
   whole block, mirroring how `iterChildrenUntil` walks a `Block` (lines 263-274):
   - **Param defaults first.** A block's `OptionalParam` defaults are evaluated before the
     body, like ordinary preceding children, so run each `OptionalParam.default_` through
     `handleChild`. (Non-optional params have no child expression to evaluate.)
   - **Then the body, as a loop body:**
     - **Selection was before the body** (continuation already found in `recv`, an arg, or
       a param default): the block may run zero or more times, so it might not run at all.
       Append `Branch(empty, block.body)` — the empty branch represents the body never
       running, the `block.body` branch represents it running.
     - **Selection is inside the body** (continuation not yet found): recurse into
       `block.body` to get the continuation *within* it, then append
       `Branch(empty, block.body)` — the empty branch represents the loop exiting, the
       `block.body` branch represents the body running again. This is the same shape as the
       `While` body case, minus the `cond` (blocks have no loop condition).

3. If a continuation was produced, return it; otherwise `break` (falls through to
   `return nullopt`).

### Sketch

```cpp
case ast::Tag::Send: {
    auto &send = ast::cast_tree_nonnull<ast::Send>(expr);
    optional<vector<ContItem>> continuation;
    auto handleChild = [&](const ast::ExpressionPtr &child) {
        if (continuation.has_value()) {
            continuation->emplace_back(&child);
        } else {
            continuation = getContinuation(child, target);
        }
    };
    handleChild(send.recv);
    for (auto &arg : send.nonBlockArgs()) {
        handleChild(arg);
    }
    if (auto *block = send.rawBlock()) {
        auto &blk = ast::cast_tree_nonnull<ast::Block>(*block);
        // Block param defaults are evaluated before the body, like ordinary children.
        for (auto &param : blk.params) {
            if (auto optArg = ast::cast_tree<ast::OptionalParam>(param)) {
                handleChild(optArg->default_);
            }
        }
        if (continuation.has_value()) {
            // Selection was before the block body; the body may or may not run afterward.
            continuation->emplace_back(&emptyTreeStorage, &blk.body);
        } else if (auto blockCont = getContinuation(blk.body, target); blockCont.has_value()) {
            // Selection is inside the block body. Treat the block as a loop body: after the
            // selection the body may run again, or the loop may exit.
            blockCont->emplace_back(&emptyTreeStorage, &blk.body);
            continuation = std::move(blockCont);
        }
    }
    if (continuation.has_value()) {
        return continuation;
    }
    break;
}
```

## Notes / rationale

- The block is walked explicitly (param defaults, then body) instead of recursing on the
  whole block, matching how `iterChildrenUntil` treats a `Block`. This keeps param defaults
  flowing through `handleChild` as ordinary preceding children, and lets only the body be
  treated as the loop body.
- No `cond` is appended (unlike `While`) because blocks have no loop condition; the
  `lastExtraStat` field is therefore left unset on the branch item.
- `Branch(empty, body)` is built with the two-arg `ContItem(branch1, branch2)` constructor,
  identical to how `While` builds its loop-back branch.

## Testing

Add LSP extract-method tests covering selections inside a block body (e.g. inside a
`.each { ... }`), verifying that variables written in the selection and re-read on a later
loop iteration of the block are correctly returned by the extracted method. Follow the test
conventions in [CLAUDE.md](CLAUDE.md): use `test/run_lsp_tests.py` and
`test/update_lsp_tests.py`, and flag any unsupported cases rather than deleting them.
