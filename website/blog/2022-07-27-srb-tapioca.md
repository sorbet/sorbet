---
id: srb-tapioca
title: Tapioca is the recommended way to generate RBIs for Sorbet
author: Jake Zimmerman
authorURL: https://twitter.com/jez_io
authorImageURL: https://avatars0.githubusercontent.com/u/5544532?s=460&v=4
---

Today we're excited to announce that the Sorbet team officially recommends using
Shopify's [Tapioca](https://github.com/Shopify/tapioca) gem for RBI generation
in projects that use Sorbet. Tapioca takes the place of the `srb rbi` family of
commands, which as of today are officially in maintenance mode. Sorbet itself
remains in active development.

<!--truncate-->

![sorbet plus tapioca](/img/sorbet-plus-tapioca.svg)

To elaborate, here's what's happening:

- The [`tapioca`](https://github.com/Shopify/tapioca) gem is now the recommended
  approach to [RBI generation](/docs/rbi) in projects that use Sorbet. This
  recommendation is reflected throughout the Sorbet docs. Tapioca replaces (and
  improves on!) functionality currently provided by the `srb rbi` command.
  Tapioca is maintained by Shopify and powers RBI generation in all of their
  Ruby codebases. We'll discuss Tapioca's strengths in more depth below.

- We're putting `srb rbi` (and by extension, certain phases of `srb init`) into
  maintenance mode. This move largely codifies what was already the case, as
  `srb rbi` has had unfixed usability problems since its creation. It will
  remain available for the foreseeable future, but may be removed at some point
  if it breaks in a way that would require substantial effort to repair.

- Stripe remains actively invested in developing the Sorbet type checker itself
  (aka, `srb tc`) as well as the runtime support libraries (`sorbet-runtime`).
  In fact, 2022 has been
  [one of the most active periods](https://github.com/sorbet/sorbet/graphs/contributors?from=2022-01-01&to=2022-07-26&type=c)
  in Sorbet's history, with nearly 700 commits from 30 different contributors so
  far this year.

As of today Sorbet's guide to
[Adopting Sorbet in an Existing Codebase](/docs/adopting) has been updated to
recommend using Tapioca. The old guide remains available in the
[repo history](https://github.com/sorbet/sorbet/blob/834c45a98cc52c2474b75fc58d06d4526888f48f/website/docs/adopting.md)
if you need to reference it.

If you'd like to start using Tapioca in a project that already uses `srb rbi`,
[see this migration guide](https://github.com/Shopify/tapioca/wiki/Migrating-to-Tapioca).

With the announcements out of the way, let's dive into some questions we imagine
you might have.

## What is Tapioca?

[Tapioca](https://github.com/Shopify/tapioca) is a Ruby gem that provides
command line tools for generating [RBI files](/docs/rbi). From its
documentation, "Tapioca surfaces types and methods from many sources that Sorbet
cannot otherwise see—such as gems, Rails and other DSLs—compiles them into RBI
files, and makes it easy for you to add gradual typing to your application."

This is largely what the `srb init` and `srb rbi` commands, (which are provided
by Sorbet itself) already do, but Tapioca does the job better. We can explain
what Tapioca does by way of comparison:

| `srb`                        | `tapioca`                                         |
| ---------------------------- | ------------------------------------------------- |
| `srb init`                   | [`tapioca init`][tapioca-init]                    |
| `srb rbi config`             | [`tapioca config`][tapioca-config]                |
| `srb rbi sorbet-typed`       | [`tapioca annotations`][tapioca-annotations]      |
| `srb rbi gems`               | [`tapioca gems`][tapioca-gems]                    |
| `srb rbi hidden-definitions` | Partially covered by [`tapioca dsl`][tapioca-dsl] |
| `srb rbi todo`               | [`tapioca todo`][tapioca-todo]                    |
| `srb rbi suggest-typed`      | Delegated to [`spoom bump`][spoom-bump]           |
| `srb rbi find-gem-rbis`      | Folded inside `tapioca gems`                      |

[tapioca-init]: https://github.com/Shopify/tapioca#getting-started
[tapioca-config]: https://github.com/Shopify/tapioca#getting-started
[tapioca-annotations]:
  https://github.com/Shopify/tapioca#pulling-rbi-annotations-from-remote-sources
[tapioca-gems]: https://github.com/Shopify/tapioca#generating-rbi-files-for-gems
[tapioca-dsl]:
  https://github.com/Shopify/tapioca#generating-rbi-files-for-rails-and-other-dsls
[tapioca-todo]:
  https://github.com/Shopify/tapioca#rbi-files-for-missing-constants-and-methods
[spoom-bump]: https://github.com/Shopify/spoom#change-the-sigil-used-in-files

You'll notice that most `srb` subcommands have a nearly equivalent (if not
exactly equivalent) command in `tapioca`, which brings us to the next question.

## What's different between `srb rbi` and `tapioca`?

From one standpoint, they're quite similar: they both use various heuristics to
generate [RBI files](/docs/rbi) before `srb tc` runs on the project.
Specifically, they both rely on Sorbet to typecheck the project itself (Tapioca
is not some sort of alternative type checker for Ruby).

Both Tapioca and `srb rbi` generate RBIs for a project's gems as well as for the
various DSLs and metaprogramming used inside a project itself.

But Tapioca has some unique benefits compared to `srb rbi`. Put simply, it's
easier to use and works better out of the box. Over the years we've seen many
times where someone new to Sorbet struggling to get `srb rbi` to do the right
thing and the suggestion to switch to `tapioca` for RBI generation gets them
unblocked.

Some of the specific benefits:

- Tapioca generally does a better job working with third-party gems. For
  example, Tapioca allows customizing how gems are loaded before attempting to
  generate an RBI for the gem.

- Tapioca has a more predictable approach to DSLs and metaprogramming. In
  particular, Tapioca has built in support for various Rails DSLs, something
  which `srb rbi` does not. Tapioca also provides an easy way to define
  [custom DSL generators](https://github.com/Shopify/tapioca#writing-custom-dsl-compilers)
  for teaching Sorbet about DSLs specific to your project.

- Tapioca will do a better job at generating actual types inside the generated
  RBI files. The custom DSL generators usually generate types in addition to
  method stubs. It will also generate `T.let` annotations for constants defined
  by the gem.

- Tapioca pulls community-driven type annotations for public gems from
  [rbi-central](https://github.com/Shopify/rbi-central) instead of from
  [sorbet-typed](https://github.com/sorbet/sorbet-typed), which `srb rbi` uses.
  The difference between the two repos is largely just about how metadata is
  stored and how
  [RBI correctness is ensured](https://github.com/Shopify/rbi-central#ci-checks).
  It should be painless to contribute any relevant `sorbet-typed` RBI files to
  `rbi-central` if your codebase depended on them.

This is only a quick summary of some of the benefits of Tapioca. For a more
detailed comparison, you can read
[this page in the Tapioca docs](https://github.com/Shopify/tapioca/wiki/How-does-Tapioca-compare-to-%60srb-rbi%60%3F).

The biggest feature that is _only_ available in `srb rbi` (and has no exact
analogue in Tapioca) is `srb rbi hidden-definitions`, which brings us to our
next question.

## What's happening with `srb rbi hidden-definitions`?

Tapioca has no exact replacement for `srb rbi hidden-definitions`. This is the
main reason why we have no active plans to remove the `srb rbi` command and its
related tooling in the immediate future.

Recall that the `hidden-definitions` subcommand works like this:

1.  Require _all_ Ruby code in the project. This includes all gems, all
    application code, potentially even all scripts.

1.  Use Ruby's reflection APIs to list all the classes, methods, and constants
    that have been defined.

1.  Compute a diff between that list of definitions and the list of definitions
    that Sorbet can see statically in the codebase (including any RBIs that
    might have already been generated).

1.  Write out an RBI containing that difference.

This tool is great when it works—you don't have to think about it!—but extremely
frustrating when it doesn't. Some ways it might not work:

- It finds too many files to require. For example, it might accidentally require
  a script that was only intended to be run manually. The `srb` tool tries to
  look for whether the file has executable permissions or a `#!` shebang at the
  top and skips running it in that case, but not all scripts have this.

  The `srb` command prints a warning that it is about to require _all_ code in
  the project, and requires a confirmation before running, but many people
  ignore this warning and then are confused when things break.

- It takes a long time. In large projects, requiring all code can take many
  minutes, quickly dwarfing how quickly Sorbet can typecheck a project (usually
  in seconds or less).

- It never generates types. Even when it can detect that a method exists at
  runtime, the generated RBI will never have types for the arguments or return
  type.

- It's not very robust to seemingly isolated failures. If there's a problem in a
  single file or a single gem, that can sometimes cause the whole process to
  abort.

For these reasons and many more, `srb rbi hidden-definitions` is one of the
tools that people struggle with the most when adopting Sorbet.

But its one selling point is that sometimes there's simply no other tool. Its
position as a "last resort" kind of tool is the main reason why we'll be keeping
`srb rbi hidden-definitions` around in maintenance mode for the foreseeable
future.

## What does "maintenance mode" mean?

Maintenance mode for `srb rbi` largely codifies what was already the case with
the `srb` tooling:

- We won't be building new features into any `srb` tool (except for `srb tc`, of
  course, which will continue to be actively improved).

- Minor changes that are easy to review will likely get reviewed (and
  automatically released), but changes that merely add new features or are hard
  to review will likely be politely rejected.

- If a future Ruby version makes a breaking change which requires substantial
  effort to fix, we may or may not have time to invest in fixing it. For this
  reason we recommend that you switch to Tapioca at your earliest convenience.

Even with thousands of changes to the Sorbet repo, the `srb rbi` command has
only been changed a handful of times in the past few years. These changes have
largely been the sort of minor, easy to review changes that we mentioned above.
The problems with `srb rbi` are pretty fundamental, and we believe that
`tapioca` solves most of them, so we won't be investing in `srb rbi` going
forwards.

Note that the source code for the `srb` command is open source and licensed
under the [Apache 2.0](https://github.com/sorbet/sorbet/blob/master/LICENSE). If
you feel strongly that any or all of the `srb rbi` commands survive, you're more
than welcome to use ideas or code from `srb rbi` into your own project! If you
do, please be aware of the
[Sorbet Trademark Policy](/docs/legal/trademark-policy). We are always happy to
list community projects on the [Community section](/en/community) of the
website.

## What happens next?

Today, we are updating various Sorbet docs pages to recommend installing Tapioca
as one of the gems you will want to install in order to get started with Sorbet
in a new codebase.

Going forward, members of Shopify will continue to maintain and improve Tapioca.
If you have questions, they're happy to chat with you in the
[#tapioca](https://sorbet-ruby.slack.com/archives/C02VD06EQ3U) channel on the
[Sorbet Slack](/slack)!

Meanwhile the Sorbet team will continue improving Sorbet. We're always eager to
hear from you in the
[#discuss](https://sorbet-ruby.slack.com/archives/CHN2L03NH) channel on Slack!

— @jez, on behalf of the Sorbet Team at Stripe

---

## Appendix: History of the `srb` and `tapioca` commands

Sorbet was initially developed in tandem with Stripe's Ruby monorepo. All
tooling that did not live in the (closed source) Sorbet repo was checked into
Stripe's monorepo. That includes all of `sorbet-runtime` (which at the time was
not even a gem, but simply a folder called `lib/types/`) as well as all of the
scripts for generating RBIs.

When we set out to open source Sorbet in the
[spring of 2019](/blog/2019/05/16/state-of-sorbet-spring-2019), we hurriedly
copied everything we thought might be useful out of Stripe's monorepo into
custom gems. After a private beta period of a few months where people were given
early access to these gems, we figured that Sorbet was good enough to share with
the world and [open sourced it](/blog/2019/06/20/open-sourcing-sorbet).

Over time, it became clear that what worked for generating RBIs in Stripe's Ruby
codebase did not necessarily work well for all Ruby codebases. Stripe's test
suite enforced stronger guarantees about what was and was not allowed in Ruby
code, which mean that, for example, requiring all code in Stripe's codebase was
trivial.

Around the same time that we announced our Sorbet private beta (over three years
ago!),
[Tapioca began development](https://github.com/Shopify/tapioca/commit/490bc863b30fb753ac4c657e6d8eaf0610e8b30f)
at Shopify. As discussed above, it took some different approaches to RBI
generation which worked better in their codebase.

After open sourcing Sorbet, the `lib/types/` folder was deleted from Stripe's
monorepo in favor of depending on the public `sorbet-runtime` gem. But the
scripts for RBI generation in Stripe's codebase were never replaced, and remain
in use today. In the three years since Sorbet has been open source, those
scripts have drifted from their open source counterparts.

Meanwhile, Shopify has been using Tapioca exclusively for those last three
years, and over time, so have other Sorbet users. We're grateful to them for
sharing Tapioca with the community and for developing the tool we wish we could
have built when open sourcing Sorbet all those years ago.
