# Contributing to Sorbet

Thanks for taking an interest in improving Sorbet!

- [I want to contribute...](#i-want-to-contribute)
  - [... an improvement to Sorbet's RBIs](#-an-improvement-to-sorbets-rbis)
  - [... a change to the sorbet.org docs website](#-a-change-to-the-sorbetorg-docs-website)
  - [... a bug fix, new feature, or refactor of Sorbet itself](#-a-bug-fix-new-feature-or-refactor-of-sorbet-itself)
- [Choosing what to work on](#choosing-what-to-work-on)
  - [Good first issues](#good-first-issues)
  - [Issues labeled `hard`](#issues-labeled-hard)
  - [Changes to user-facing syntax and APIs](#changes-to-user-facing-syntax-and-apis)
  - [Other kinds of changes](#other-kinds-of-changes)
- [Testing](#testing)
- [Review expectations](#review-expectations)
- [Design Proposals: Scoping large features](#design-proposals-scoping-large-features)


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


## ... a bug fix, new feature, or refactor of Sorbet itself

We are **very particular** about changes to Sorbet itself.

**Introducing yourself** and your intention to change Sorbet is the best way to get started. You can always find us in in the [#internals](https://sorbet-ruby.slack.com/archives/CFT8Y4909) channel of the [Sorbet Slack](https://sorbet.org/slack).

It puts us in an awkward position when the first time we hear from you is after you've opened a PR with a few hundred or few thousand lines of new code written in a vacuum, and but the PR stems from a fundamental misunderstanding that requires going back to the drawing board!

We love getting to chat with people looking to make changes to Sorbet, and the eventual rate of success is much higher when we can help start people down the right direction.

Please, please, please **introduce yourself** before making large changes!

Asking on Slack is typically better than commenting on old issues, because it can be hard to keep up with discussions on GitHub issues (there are so many, and people sometimes get into lengthy back-and-forth's which drown out productive discussions). You're welcome to try asking on the GitHub issue first, but consider switching to Slack if you don't get a response within a few days.


# Choosing what to work on


## Good first issues

Issues labeled [`good first issue`](https://github.com/sorbet/sorbet/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22) in the Sorbet issue tracker are the best way to start when you're new to Sorbet.

Every one of these issues has been hand-curated by a member of the Sorbet team:

- They're changes we definitely want fixed!
- They're already "scoped" meaning that it's unlikely unknown blockers will come up in the course of implementing the feature.
- They're bite-sized: it would take someone on the Sorbet team anywhere from a few hours to a few days to fully build the feature.

If you [introduce yourself](#https://git.corp.stripe.com/gist/#-a-bug-fix-new-feature-or-refactor-of-sorbet-itself) and say you want to work on a `good first issue`, we'll usually say hi, offer help if you want it, and otherwise be eager to see what you come up with!


## Issues labeled `hard`

Please do not attempt issues labeled `hard`. Hard issues are not a "challenge" to step up to once you've become familiar enough with Sorbet. Rather, issues labeled hard track somewhat fundamentally hard issues that would require near-complete rewrites of parts or all of Sorbet.

In rare cases, members of the Sorbet team might have hypotheses of possible ways to approach the problem, but merely validating the hypothesis or accurately describing the hypothesis well enough would amount to outright implementing the feature. That is: even in cases where we might think a hard issue is possible, it's _also_ hard to communicate the hardness in a satisfying way to an external contributor.

In most cases, hard issues are suspected to never be fixed. Because of the expected return on investment in thinking about these issues, the Sorbet team nearly always prioritizes other, more achievable work.

We track `hard` issues anyways, despite all of this, for two reasons:

1.  To communicate that someone has at least recognized the problem, and possibly even thought about solutions. This allows members of the community to be aware of the context.

2.  To remind ourselves of the existence of the problem, in case we ever revisit it.

From time to time, [we do close `hard` issues](https://github.com/sorbet/sorbet/issues/?q=is%3Aissue%20state%3Aclosed%20label%3Ahard%20sort%3Aupdated-desc). In almost all of these cases, the issue was closed because someone on the Sorbet team had a spark of inspiration in passing which reframed the problem as an easy (or at least: tractable) one. If you think you have such an idea about a hard problem, feel free to reach out.


## Changes to user-facing syntax and APIs

Changes to user-facing syntax **always** require a [design proposal](#design-proposals-scoping-large-features). This includes changes to `sig` and associated builder methods, `T.let`, type syntax, `# typed:`, any `sorbet-runtime` API which is either directly exposed (e.g. `T::Types`, `T::Configuration`, etc.) or indirectly exposed (e.g., what's stored on runtime `Signature` objects).

[See below](#design-proposals-scoping-large-features) for more.


## Other kinds of changes

Please [introduce yourself](#https://git.corp.stripe.com/gist/#-a-bug-fix-new-feature-or-refactor-of-sorbet-itself) and we'll be happy to help you figure out what the best next step would be.

There are many kinds of changes that Sorbet sees, from tiny bug fixes, error message improvements, and variable name changes, to large new features and backwards-incompatible type system improvements—it's hard to give advice for all of them. If the kind of change you want to make doesn't align with any of the previous sections, please reach out.

Some common next steps might be

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


# Design Proposals: Scoping large features

- [ ] @jez Write this section

<!-- vim:tw=0
-->
