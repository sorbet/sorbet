We welcome contributions from the community. This doc describes the process to contribute patches and the general guidelines we expect contributors to follow.

# Communication
* Before starting work on a major feature, please reach out to us via GitHub, Slack,
  email, etc. We will make sure no one else is already working on it and ask you to open a
  GitHub issue.
* A "major feature" is defined as any change that is > 100 LOC altered (not including tests), or
  changes any user-facing behavior. We will use the GitHub issue to discuss the feature and come to
  agreement. This is to prevent your time being wasted, as well as ours
* Small patches and bug fixes don't need prior communication.
* Some good tasks to get started with are [available in the issue tracker](https://github.com/stripe/sorbet/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22).

# Release cadence
* Currently we are targeting approximately quaterly official releases. We may change this based
  on customer demand.
* In general, master is assumed to be release candidate quality at all times for documented
  features. For undocumented or clearly under development features, use caution or ask about
  current status when running master. Stripe runs master in production, typically deploying every
  week.
* We currently provide binary packages available via [releases page](https://github.com/stripe/sorbet/releases).

# Writing Documentation
Documentation improvements are very welcome. The source of [stripe.dev/sorbet](http://stripe.github.io/sorbet/) is located in website/ in the tree.

# Testing
* Our CI runs all tests with [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) and [ASan](https://clang.llvm.org/docs/AddressSanitizer.html). You can run them locally by passing `--config=sanitize` when running tests
* We expect all assumptions made when writing code to be spelled explicitly via `ENFORCE(myAssumption);` statements.
* Some characteristics of good tests:
    * includes comments: what is being tested and why?
    * be minimal, deterministic, stable (unaffected by irrelevant changes), easy to understand and review
    * have minimal dependencies: a typechecker bug test should not depend on, e.g. the Ruby standard library

# Submitting a PR
* Create small PRs that are narrowly focused on addressing a single concern. We often receive PRs that are trying to fix several things at a time, but if only one fix is considered acceptable, nothing gets merged and both author's & review's time is wasted. Create more PRs to address different concerns and everyone will be happy.
* Bug fixes should include regression tests -- in the same commit as the fix. If testing isn't feasible, the commit message should explain why.
* New features and enhancements must be motivated by adding common usage as tests.
* Follow the Boy Scout Rule: "Always leave the code behind in a better state than you found it"
* We will **not** merge any PR that is not passing tests.
* Your PR description should have details on what the PR does. If it fixes an existing issue it should end with "Fixes #XXX".
* When all of the tests are passing and all other conditions described herein are satisfied, a maintainer will be assigned to review and merge the PR.
* We expect that once a PR is opened, it will be actively worked on until it is merged or closed. We reserve the right to close PRs that are not making progress. This is generally defined as no changes for 7 days. Obviously PRs that are closed due to lack of activity can be reopened later. Closing stale PRs helps us to keep on top of all of the work currently in flight.
* Please consider joining the [sorbet-slack](https://sorbet-ruby.slack.com).

# PR review policy for maintainers
* Typically we try to turn around reviews within two business days.
* It is generally expected that at lease a single maintainer should review every PR.
* If there is a question on who should review a PR please discuss in Slack.
* Anyone is welcome to review any PR that they want, whether they are a maintainer or not.
* As PR's are merged, they are tested against Stripe internal codebase and are pushed in squashed form to GitHub.
* Please **clean up the title and body** before merging. By default, our merge bot fills the squash merge title with the original title, and the commit body with every individual commit from the PR.
