# Contributing to Sorbet

Thanks for taking an interest in improving Sorbet!

- [I want to contribute...](#i-want-to-contribute)
  - [... an improvement to Sorbet's RBIs](#-an-improvement-to-sorbets-rbis)
  - [... a change to the sorbet.org docs website](#-a-change-to-the-sorbetorg-docs-website)
  - [... a fix or feature already tracked in an issue](#-a-fix-or-feature-already-tracked-in-an-issue)
  - [... a fix or feature that's not been discussed before](#-a-fix-or-feature-thats-not-been-discussed-before)
- [Design Proposals: Scoping large features](#design-proposals-scoping-large-features)
  - [How to share a design proposal](#how-to-share-a-design-proposal)
  - [Tips](#tips)
- [Choosing what to work on](#choosing-what-to-work-on)
  - [Good first issues](#good-first-issues)
  - [Issues labeled `hard`](#issues-labeled-hard)
  - [Changes to user-facing syntax and APIs](#changes-to-user-facing-syntax-and-apis)
  - [Other kinds of changes](#other-kinds-of-changes)
- [Testing](#testing)
- [Review expectations](#review-expectations)


# I want to contribute...


## ... an improvement to Sorbet's RBIs

Great, thanks!

This is by far the most common kind of external contribution to Sorbet, and is typically very low touch.

Please read these sections of the Sorbet docs:

→ [It looks like Sorbet's types for the stdlib are wrong](https://sorbet.org/docs/faq#it-looks-like-sorbets-types-for-the-stdlib-are-wrong)<br>
→ [Versioning for standard library RBIs](https://sorbet.org/docs/rbi#versioning-for-standard-library-rbis)

**Do I need to write tests?**

No, but you can if you want to. All changes (RBI changes included) are tested by running a one-off build of Sorbet using the build produced from the PR against Stripe's massive Ruby codebase.

If you want to write tests, find an example test file in `test/testdata/rbi/` and either add to it or create a new such file. See the [README](README.md#writing-tests) for how to write these tests.


## ... a change to the sorbet.org docs website

Awesome!

Please refer to our [website style guide](website/style-guide.md).


## ... a fix or feature already tracked in an issue

If you've already picked out an existing issue to work on (especially a [good first issue](#good-first-issues)!) the best next step is to **introduce yourself** and your intention to start working on that issue.

Drop a quick message in the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) channel of the [Sorbet Slack](https://sorbet.org/slack). Giving a heads up to the Sorbet team on Slack is generally better than commenting on old issues—it'll be more likely that the whole team sees your message. Comments on issues are usually better for logging information or context for the next person who will work on the issue. You're welcome to try asking on the GitHub issue first, but consider switching to Slack if you don't get a response within a few days.

Someone from the Sorbet team will ask whether you already have a solution in mind, or give you hints or tips for what we think the problem might be, and we'll go from there.

Occasionally, we'll realize that the issue either has already been fixed, is harder than expected, or isn't actually scoped well enough (and would need a [Design Proposal](#design-proposals-scoping-large-features)). Introducing your intention to fix an issue lets someone from the Sorbet team catch these things before they become a time sink.


## ... a fix or feature that's not been discussed before

Please follow the [Design Proposals: Scoping large features](#design-proposals-scoping-large-features) section below. We'd love for you to follow the template for proposing new features, so that we can review what your proposing before committing to a particular solution.

We love when people have ideas for how to change Sorbet, because that enthusiasm for making Sorbet better energizes us as well! But we are **very particular** about changes to Sorbet itself, because Sorbet is something that tons of people use and love every day. We need to make sure that changes preserve the good parts and play to Sorbet's strengths.

Usually, we want to chat briefly so that we can:

- Agree on a definition of the problem and motivation
- Have a discussion about the solution space and potential alternatives
- Double-check implementation considerations that preclude otherwise-good solutions
- Estimate difficulty and other nuance in the change itself

Successful contributions happen tend to come when we've helped start people down the right direction.

It's fine to build a prototype of a change _before_ reaching out to us if you'd like, but it puts us in an awkward position when the first time we hear from you is after you've opened a PR with a few hundred or few thousand lines of new "ready for review" code written in a vacuum, but the PR stems from a fundamental misunderstanding about something that requires going back to the drawing board.

# Design Proposals: Scoping large features

Large changes to Sorbet should be intentional and planned.

Before making large changes, we should be able to answer these questions:

- What is the problem?

  Be as **specific** and as **concrete** as possible. The point of a design proposal is to make sure we've thought things through from all angles. Reviewers will suggest all sorts of alternative solutions. If the problem statement is misrepresented, inaccurate, or incomplete, these alternatives will not actually solve the problem, and you'll have to further explain the problem.

- What's the best solution, and why?

  The proposal should pick one solution and defend it. You should be the biggest advocate of your feature: justify and motivate why this particular approach is better than all the rest. In particular, it usually makes sense to **state hypothetical alternative solutions**, and compare why the chosen solution is better.

- How do other projects (type checkers, compilers, IDEs, language runtimes, etc.) solve this problem?

  Research and summarize prior art. For example, is there a similar feature in TypeScript or some other widely-used gradual type system? Does another IDE have a comparable editor feature? How do other language servers implement this LSP feature? Etc. Thorough, compelling background and prior art is one of the best ways to argue in favor of your proposal. It shows that you've invested time and effort to come up with a feature that improves on existing work, rather than invents something entirely without precedent.

- How does this feature interact or overlap with other Sorbet features?

  Having multiple, overlapping ways of doing something causes confusion, because people will ask, "which should I use?" For example of what not to do, see to what lengths the docs have to go through to explain the [difference between `T.class_of` and `T::Class`]. Lengthy, confusing explanations are what we want to avoid in new features.

  Most of the time, new type system features are complicated by how they interact with subtyping, with generics, or with both of those. Be sure to call out such interactions for changes to the type system itself. For changes to `sorbet-runtime`, be aware that [Sorbet erases generics at runtime]. Runtime changes cannot rely on the runtime checks having access to the same information that the static checker does.

[difference between `T.class_of` and `T::Class`]: https://sorbet.org/docs/class-of#tclass-vs-tclass_of
[Sorbet erases generics at runtime]: https://sorbet.org/docs/generics#generics-and-runtime-checks

## How to share a design proposal

The questions above are captured in a template:

→ [design-proposal-template.md](docs/design-proposal-template.md)

To create a new proposal:

1.  Open that file in GitHub, and click "Copy raw file" at the top of the file

1.  Visit <https://hackmd.io/new> to create a new, blank doc, or open a GitHub Issue with the proposal template.

    HackMD is a free, real-time collaborative editor that supports Markdown and in-line commenting, and is preferred for lengthier proposals or those which would benefit from multiple, threaded discussions. You may also open a GitHub Issue with the proposal template, especially for short proposals. 

1.  Follow the instructions in the template to fill it out.

1.  Share a link to your proposal in the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) channel on the [Sorbet Slack](https://sorbet.org/slack).

## Tips

Some further tips for good design proposals:

- Prioritize **semantics** over syntax. Once we've figured out the ideal semantics for a given feature, we can then align on the best syntax for it.

- Read (and really think about) Sorbet's [user-facing design principles]. They will come up repeatedly while people review your design proposal. If you're proposing to go against one of these principles, you should explicitly call it out **and** justify why. If you don't, it will just be the first comment someone makes on your proposal.

- It's fine to build prototypes and proof-of-concept PRs before writing a design proposal, but know that proposals will be judged on their own merits. Saying "this approach is best because it's what the prototype already does, and doing more work would be harder" does not support itself. But saying, "I built the prototype three ways, tried all of them, and this one was best because the prototypes showed me X" does meaningfully contribute to the discussion.

[user-facing design principles]: README.md#sorbet-user-facing-design-principles

# Choosing what to work on


## Good first issues

Issues labeled [`good first issue`](https://github.com/sorbet/sorbet/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22) in the Sorbet issue tracker are the best way to start when you're new to Sorbet.

Every one of these issues has been hand-curated by a member of the Sorbet team:

- They're changes we definitely want fixed!
- They're already "scoped" meaning that it's unlikely unknown blockers will come up in the course of implementing the feature.
- They're bite-sized: it would take someone on the Sorbet team anywhere from a few hours to a few days to fully build the feature.

If you [introduce yourself](#https://git.corp.stripe.com/gist/#-a-fix-or-feature-already-tracked-in-an-issue) and say you want to work on a `good first issue`, we'll usually say hi, offer help if you want it, and otherwise be eager to see what you come up with!


## Issues labeled `hard`

Please do not attempt issues labeled `hard`. Hard issues are not a "challenge" to step up to once you've become familiar enough with Sorbet. Rather, issues labeled hard track somewhat fundamentally hard issues that would require near-complete rewrites of parts or all of Sorbet.

In rare cases, members of the Sorbet team might have hypotheses of possible ways to approach the problem, but merely validating the hypothesis or accurately describing the hypothesis well enough would amount to outright implementing the feature. That is: even in cases where we might think a hard issue is possible, it's _also_ hard to communicate the hardness in a satisfying way to an external contributor.

In most cases, hard issues are suspected to never be fixed. Because of the expected return on investment in thinking about these issues, the Sorbet team nearly always prioritizes other, more achievable work.

We track `hard` issues anyways, despite all of this, for two reasons:

1.  To communicate that someone has at least recognized the problem, and possibly even thought about solutions. This allows members of the community to be aware of the context.

2.  To remind ourselves of the existence of the problem, in case we ever revisit it.

From time to time, [we do close `hard` issues](https://github.com/sorbet/sorbet/issues/?q=is%3Aissue%20state%3Aclosed%20label%3Ahard%20sort%3Aupdated-desc). In almost all of these cases, the issue was closed because someone on the Sorbet team had a spark of inspiration in passing which reframed the problem as an easy (or at least: tractable) one. If you think you have such an idea about a hard problem, feel free to reach out.


## Changes to user-facing syntax and APIs

Changes to user-facing syntax **always** require a [design proposal](#design-proposals-scoping-large-features). This includes changes to `sig` and associated builder methods, `T.let`, type syntax, `# typed:`, command line options, any `sorbet-runtime` API which is either directly exposed (e.g. `T::Types`, `T::Configuration`, etc.) or indirectly exposed (e.g., what's stored on runtime `Signature` objects).

[See below](#design-proposals-scoping-large-features) for more.


## Other kinds of changes

Please [introduce yourself](#https://git.corp.stripe.com/gist/#-a-bug-fix-new-feature-or-refactor-of-sorbet-itself) and we'll be happy to help you figure out what the best first step for making a change would be.

There are many kinds of changes that Sorbet sees, from tiny bug fixes, error message improvements, and variable name changes, to large new features and backwards-incompatible type system improvements—it's hard to give advice for all of them. If the kind of change you want to make doesn't align with any of the previous sections, please reach out.

Some common next steps might be:

- The change is a change we definitely want, but it needs a [design proposal](#design-proposals-scoping-large-features) before work should begin.

- The change has been attempted before, meaning that we can help avoid any pitfalls this time before you get started.

- We suspect the change will introduce an onerous number of errors in existing Sorbet projects (even if the feature itself would have been good to implement in a vacuum). These changes can be hard to navigate, and can end with us opting to decline accepting an otherwise-good feature.

- The change you're proposing is not a change we want, even if someone else implemented it on their own time. We may not want changes for any number of considerations: architectural, user experience, conflicts with future plans, etc. Please recognize that not all features are features we want.

When it's unclear, ask 🙂


# Testing

You will be asked to write tests (except for RBI and website changes). There is extensive documentation about how to write tests in the [README](README.md#writing-tests).

CI will not run your tests until someone from the Sorbet team manually starts the tests on your build. You can run the tests locally, or ask in the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) or [#buildkite-blocked](https://sorbet-ruby.slack.com/archives/CMYPHC6AJ) channels on the [Sorbet Slack](https://sorbet.org/slack) to have someone start the tests for your commits.

For changes that we suspect might have introduce new, backwards-incompatible type errors on existing Sorbet projects, a member of the Sorbet team will run a one-off run of Sorbet over Stripe's codebase using the changes in your PR. You will not be able to see this test result—a member of the Sorbet team will summarize the results for you.


# Review expectations

Someone from the Sorbet team will be automatically assigned to review your change.

For most changes, especially RBI changes, we review the change the same business day.

For larger changes, we try to get around to reviewing the PR within one week.

If you feel like your PR has gotten lost and it's been open for more than a week, please reach out on the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) channel on the [Sorbet Slack](https://sorbet.org/slack).


<!-- vim:tw=0
-->
