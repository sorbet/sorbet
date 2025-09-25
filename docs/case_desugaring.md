# Desugaring Classic `case ... when` with Prism

This guide walks through how Sorbet desugars a Ruby `case` statement when the
Prism parser is enabled. It is written for contributors who are getting
comfortable with Prism’s translator and the `NodeWithExpr` pattern.

We will:

1. Recap what the *legacy* desugar in `ast/desugar/PrismDesugar.cc` used to do.
2. Show the new translator-based implementation in `parser/prism/Translator.cc`.
3. Compare the Ruby input to the Sorbet AST output so you can see the end-to-end
   transformation.

Throughout the document we’ll use the same running example:

```ruby
case value
when 1, 2
  :small
when *others
  :many
else
  :unknown
end
```

Sorbet ultimately wants to turn that into this AST (simplified):

```
Magic.caseWhen(
  value,                                      # predicate
  3,                                          # number of patterns
  1, 2, Magic.checkMatchArray(value, others), # each `when` pattern
  :small, :many, :unknown                     # bodies + else
)
```

When `preserveConcreteSyntax` is `false`, we instead build nested `if` nodes
that model Ruby’s runtime control flow. Both shapes are described below.

---

## 1. Legacy Desugar (before our change)

Prism originally emitted a plain `parser::Case` node and left all of the real
work to `node2TreeImpl` inside
[`ast/desugar/PrismDesugar.cc`](../ast/desugar/PrismDesugar.cc). The relevant
portion looked like this (comments added for clarity):

```cpp
[&](parser::Case *case_) {
    if (dctx.preserveConcreteSyntax) {
        // Branch A: build Magic.caseWhen(...)
        Send::ARGS_store args;

        // 1. Push the predicate expression, e.g. `value`
        args.emplace_back(node2TreeImpl(dctx, case_->condition));

        // 2. Collect every pattern from every `when`
        Send::ARGS_store patterns;
        Send::ARGS_store bodies;
        for (auto &whenNode : case_->whens) {
            auto when = parser::NodeWithExpr::cast_node<parser::When>(whenNode.get());
            for (auto &pattern : when->patterns) {
                patterns.emplace_back(node2TreeImpl(dctx, pattern));
            }
            bodies.emplace_back(node2TreeImpl(dctx, when->body));
        }

        // 3. Append the else-body, even if it is nil
        bodies.emplace_back(node2TreeImpl(dctx, case_->else_));

        // 4. Stitch it together: <predicate>, <num-patterns>, <patterns>, <bodies>
        args.emplace_back(MK::Int(locZeroLen, patterns.size()));
        move(patterns.begin(), patterns.end(), back_inserter(args));
        move(bodies.begin(), bodies.end(), back_inserter(args));

        // 5. Emit Magic.caseWhen(...)
        result = MK::Send(loc, MK::Magic(locZeroLen), core::Names::caseWhen(),
                          locZeroLen, args.size(), move(args));
        return;
    }

    // Branch B: build nested `if` nodes (when we don't keep concrete syntax)
    ExpressionPtr assign;
    auto temp = core::NameRef::noName();
    core::LocOffsets cloc;

    if (case_->condition != nullptr) {
        // Cache the predicate in a fresh local so we only evaluate it once
        cloc = case_->condition->loc;
        temp = dctx.freshNameUnique(core::Names::assignTemp());
        assign = MK::Assign(cloc, temp, node2TreeImpl(dctx, case_->condition));
    }

    // Start with the else-branch ...
    ExpressionPtr res = node2TreeImpl(dctx, case_->else_);

    // ... then fold each `when` backwards into nested `if`s.
    for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
        auto when = parser::NodeWithExpr::cast_node<parser::When>(it->get());
        ExpressionPtr cond;

        for (auto &pattern : when->patterns) {
            ExpressionPtr test;
            if (parser::NodeWithExpr::isa_node<parser::Splat>(pattern.get())) {
                // splat pattern => Magic.checkMatchArray(predicate, pattern)
                ENFORCE(temp.exists(), "splats need something to test against");
                auto splat = node2TreeImpl(dctx, pattern);
                auto local = MK::Local(cloc, temp);
                test = MK::Send2(splat.loc(), MK::Magic(loc), core::Names::checkMatchArray(),
                                 locZeroLen, move(local), move(splat));
            } else {
                // regular pattern => pattern === predicate
                auto ctree = node2TreeImpl(dctx, pattern);
                if (temp.exists()) {
                    auto local = MK::Local(cloc, temp);
                    test = MK::Send1(ctree.loc(), move(ctree), core::Names::tripleEq(),
                                     locZeroLen, move(local));
                } else {
                    test = move(ctree);
                }
            }

            // Combine the tests with a right-associative OR tree
            cond = cond == nullptr ? move(test)
                                    : MK::If(test.loc(), move(test), MK::True(test.loc()), move(cond));
        }

        // Nest the if: if (cond) then body else previous_result
        res = MK::If(when->loc, move(cond), node2TreeImpl(dctx, when->body), move(res));
    }

    if (assign != nullptr) {
        res = MK::InsSeq1(loc, move(assign), move(res));
    }
    result = move(res);
}
```

**Key takeaways:**

- Every child node is re-translated on the spot by calling `node2TreeImpl`.
- There are two shapes: `Magic.caseWhen(...)` or nested `if`/`Magic.checkMatchArray`.
- `parser::Case`, `parser::When`, and friends are just containers; they don’t
  hold the final AST until this function runs.

---

## 2. Translator-Based Desugar (current state)

To avoid a second pass, we now build the final AST inside
`parser/prism/Translator.cc` when we encounter the Prism `PM_CASE_NODE`. The
skeleton of the new case is below (trimmed to highlight the important pieces):

```cpp
case PM_CASE_NODE: {
    auto caseNode = down_cast<pm_case_node>(node);

    // 1. Translate predicate, whens, and else as parser nodes
    auto predicate = translate(caseNode->predicate);
    auto prismWhenNodes = absl::MakeSpan(caseNode->conditions.nodes,
                                         caseNode->conditions.size);

    NodeVec whenNodes;
    whenNodes.reserve(prismWhenNodes.size());
    for (auto *whenPrism : prismWhenNodes) {
        auto *whenNode = down_cast<pm_when_node>(whenPrism);
        auto whenLoc = translateLoc(whenNode->base.location);

        NodeVec patternNodes;
        translateMultiInto(patternNodes,
                           absl::MakeSpan(whenNode->conditions.nodes,
                                          whenNode->conditions.size));

        auto statementsNode = translateStatements(whenNode->statements);

        // Wrap each `when` in a NodeWithExpr *even if* the expr is empty for now.
        // This lets the parent consume takeDesugaredExpr() later.
        whenNodes.emplace_back(make_node_with_expr<parser::When>(
            MK::EmptyTree(), whenLoc, move(patternNodes), move(statementsNode)));
    }

    auto elseClause = translate(up_cast(caseNode->else_clause));

    // 2. Bail out if any child is missing a desugared expression
    if (!directlyDesugar || !hasExpr(predicate, elseClause) || !hasExpr(whenNodes)) {
        return make_unique<Case>(location, move(predicate), move(whenNodes), move(elseClause));
    }

    // 3a. preserveConcreteSyntax => build Magic.caseWhen(...)
    if (preserveConcreteSyntax) {
        auto locZero = location.copyWithZeroLength();
        ast::Send::ARGS_store args;

        // predicate (value or EmptyTree)
        args.emplace_back(predicate == nullptr ? MK::EmptyTree()
                                               : predicate->takeDesugaredExpr());

        // number of patterns
        size_t totalPatterns = 0;
        for (auto &wrapped : whenNodes) {
            auto whenNode = parser::NodeWithExpr::cast_node<parser::When>(wrapped.get());
            totalPatterns += whenNode->patterns.size();
        }
        args.emplace_back(MK::Int(locZero, totalPatterns));

        // individual pattern expressions
        for (auto &wrapped : whenNodes) {
            auto whenNode = parser::NodeWithExpr::cast_node<parser::When>(wrapped.get());
            for (auto &pattern : whenNode->patterns) {
                args.emplace_back(pattern == nullptr ? MK::EmptyTree()
                                                     : pattern->takeDesugaredExpr());
            }
        }

        // bodies for each when + the else body
        for (auto &wrapped : whenNodes) {
            auto whenNode = parser::NodeWithExpr::cast_node<parser::When>(wrapped.get());
            args.emplace_back(whenNode->body == nullptr ? MK::EmptyTree()
                                                        : whenNode->body->takeDesugaredExpr());
        }
        args.emplace_back(elseClause == nullptr ? MK::EmptyTree()
                                                : elseClause->takeDesugaredExpr());

        auto expr = MK::Send(location, MK::Magic(locZero), core::Names::caseWhen(),
                             locZero, args.size(), move(args));

        return make_node_with_expr<Case>(move(expr), location, move(predicate),
                                          move(whenNodes), move(elseClause));
    }

    // 3b. Otherwise build nested `if` nodes
    core::NameRef tempName = core::NameRef::noName();
    core::LocOffsets predicateLoc;
    ExpressionPtr assignExpr;

    if (predicate != nullptr) {
        predicateLoc = predicate->loc;
        tempName = nextUniqueDesugarName(core::Names::assignTemp());
        assignExpr = MK::Assign(predicateLoc, tempName, predicate->takeDesugaredExpr());
    }

    ExpressionPtr resultExpr = elseClause == nullptr ? MK::EmptyTree()
                                                     : elseClause->takeDesugaredExpr();

    for (auto it = whenNodes.rbegin(); it != whenNodes.rend(); ++it) {
        auto whenNode = parser::NodeWithExpr::cast_node<parser::When>(it->get());
        ExpressionPtr condExpr;

        for (auto &patternNode : whenNode->patterns) {
            auto patternExpr = patternNode == nullptr ? MK::EmptyTree()
                                                      : patternNode->takeDesugaredExpr();
            auto patternLoc = patternExpr.loc();

            ExpressionPtr testExpr;
            if (parser::NodeWithExpr::isa_node<parser::Splat>(patternNode.get())) {
                ENFORCE(tempName.exists(), "splats need something to test against");
                auto local = MK::Local(predicateLoc, tempName);
                testExpr = MK::Send2(patternLoc, MK::Magic(location.copyWithZeroLength()),
                                     core::Names::checkMatchArray(), patternLoc.copyWithZeroLength(),
                                     move(local), move(patternExpr));
            } else if (tempName.exists()) {
                auto local = MK::Local(predicateLoc, tempName);
                testExpr = MK::Send1(patternLoc, move(patternExpr), core::Names::tripleEq(),
                                     patternLoc.copyWithZeroLength(), move(local));
            } else {
                testExpr = move(patternExpr);
            }

            condExpr = condExpr == nullptr ? move(testExpr)
                                           : MK::If(testExpr.loc(), move(testExpr),
                                                    MK::True(testExpr.loc()), move(condExpr));
        }

        auto thenExpr = whenNode->body == nullptr ? MK::EmptyTree()
                                                  : whenNode->body->takeDesugaredExpr();
        resultExpr = MK::If(whenNode->loc, move(condExpr), move(thenExpr), move(resultExpr));
    }

    if (assignExpr != nullptr) {
        resultExpr = MK::InsSeq1(location, move(assignExpr), move(resultExpr));
    }

    return make_node_with_expr<Case>(move(resultExpr), location, move(predicate),
                                      move(whenNodes), move(elseClause));
}
```

### What changed?

- **We wrap `parser::When` in a `NodeWithExpr` immediately.** Even though a `when`
  by itself doesn’t know how to become Ruby code, wrapping it lets the parent call
  `takeDesugaredExpr()` and stay in the “everything returns a NodeWithExpr” world.
- **We only desugar once every child node already carries an expression.** The
  `hasExpr(...)` guard guarantees that if any part is still waiting for a later
  pass, we return the legacy nodes untouched and let `PrismDesugar` fall back.
- **The AST we create is byte-for-byte equivalent to the legacy version.** The
  code in the translator mirrors the shape that used to be built in
  `node2TreeImpl`, so tests behave the same but we skip an extra traversal.

---

## 3. Before & After: putting it together

Using the sample Ruby snippet at the top, here’s the flow with the new
translator:

1. Prism produces `pm_case_node`, `pm_when_node`, etc.
2. `Translator::translate(pm_case_node)`:
   - Translates each `pm_when_node` into a `parser::When` wrapped in a
     `NodeWithExpr` (the wrapped expression is initially `EmptyTree`).
   - When every child reports `hasDesugaredExpr()`, the translator constructs
     either:
     - `Magic.caseWhen(value, 3, 1, 2, Magic.checkMatchArray(value, others), :small, :many, :unknown)`; or
     - A nested `if` chain with optional predicate assignment, matching the
       old desugar phase exactly.
   - The final result is a `NodeWithExpr<parser::Case>` containing that AST.
3. `node2TreeImpl` hits the `parser::Case` clause and immediately calls
   `desugaredByPrismTranslator(case_)`, which simply asserts that the translator
   already filled in the expression.

If some child failed to desugar (for example, if a `when` pattern contained a
node we haven’t implemented yet), the translator falls back to the plain
`parser::Case` and the legacy branch in `PrismDesugar` kicks in. That keeps the
system safe while we migrate the remaining node types.

---

## 4. FAQ

- **Why keep returning `parser::When` at all?** Because the translator still has
  to hand back something derived from `parser::Node`. Wrapping the legacy node in
  a `NodeWithExpr` gives downstream callers a uniform way to access the desugared
  expression while preserving compatibility with code that still inspects the
  legacy structure (printer, validator, etc.).

- **Do the new paths run in tests if individual `when` nodes aren’t desugared?**
  Yes. Even though `PM_WHEN_NODE` itself doesn’t produce a final AST, its
  children (patterns, bodies) do. Once they’re ready, the `case` code we saw
  above assembles the final expression and returns a `NodeWithExpr<Case>`. The
  regression suites (`bazel test //test:prism_regression`) exercise both the
  `Magic.caseWhen` and nested `if` paths.

- **What’s next?** As we migrate more nodes, the goal is for every branch to
  follow the same pattern: translate Prism data directly, gate on `hasExpr`, and
  return a `NodeWithExpr` so the desugar phase simply delegates to
  `desugaredByPrismTranslator`.

Hopefully this gives you a clearer picture of what moved where and why. If
anything is still fuzzy, drop a question on the PR or in the comments here and
we can expand the examples further.
