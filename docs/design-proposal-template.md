# TODO: My Proposal

## Instructions

- [ ] Click "Copy raw file" on this template
- [ ] Visit <https://hackmd.io/new> to create a new, blank doc
  - You should not need to sign in to do this, but you can if you like
- [ ] Paste the copied template contents
- [ ] Fill out the rest of the template
- [ ] Ensure the sharing settings match this:
  - `Read: Everyone`
  - `Write: Signed-in users` **or** `Write: Everyone`
  - Engagement control: `Commenting`, `Suggest edit`, `Emoji reply`
- [ ] Share a link to this doc in the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) channel on the [Sorbet Slack](https://sorbet.org/slack)

You MAY deviate from this template if you think it improves the clarity of the
proposal. For example: you may consider changing the section titles to one-line
summaries of the problem, solution, etc. You may add additional sections or sub
sections.

For more writing tips, see <https://blog.jez.io/dont-bury-the-lede/>

**Note**: Most of the writing prompts in this template are commented out—you may
want to consider the "Raw" view when reading on GitHub.

- - - - -

- **Author(s)**: TODO
- **Organization**: TODO (optional)

## Problem

<!--
  What is the problem?

  Be as SPECIFIC and as CONCRETE as possible. The point of a design proposal is
  to make sure we've thought things through from all angles. Reviewers will
  suggest all sorts of alternative solutions. If the problem statement is
  misrepresented, inaccurate, or incomplete, these alternatives will not
  actually solve the problem, and you'll have to further explain the problem.
-->

TODO

## Proposed solution

<!--
  What's the best solution, and why?

  The proposal should pick one solution and defend it. You should be the biggest
  advocate of your feature: justify and motivate why this particular approach is
  better than all the rest. In particular, it usually makes sense to **state
  hypothetical alternative solutions**, and compare why the chosen solution is
  better.
-->

TODO

### Alternatives considered

TODO

## Prior art

<!--
  How do other projects (type checkers, compilers, IDEs, language runtimes,
  etc.) solve this problem?

  Research and summarize prior art. For example, is there a similar feature in
  TypeScript or some other widely-used gradual type system? Does another IDE
  have a comparable editor feature? How do other language servers implement this
  LSP feature? Etc. Thorough, compelling background and prior art is one of the
  best ways to argue in favor of your proposal. It shows that you've invested
  time and effort to come up with a feature that improves on existing work,
  rather than invents something entirely without precedent.
-->

TODO

## Edge cases and interactions

<!--
  How does this feature interact or overlap with other Sorbet features?

  Having multiple, overlapping ways of doing something causes confusion, because
  people will ask, "which should I use?" For example of what not to do, see to
  what lengths the docs have to go through to explain the [difference between
  `T.class_of` and `T::Class`]. Lengthy, confusing explanations are what we want
  to avoid in new features.

  Most of the time, new type system features are complicated by how they
  interact with subtyping, with generics, or with both of those. Be sure to call
  out such interactions for changes to the type system itself. For changes to
  `sorbet-runtime`, be aware that [Sorbet erases generics at runtime]. Runtime
  changes cannot rely on the runtime checks having access to the same
  information that the static checker does.
-->

TODO

