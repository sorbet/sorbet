---
id: metrics
title: Tracking Adoption with Metrics
sidebar_label: Tracking Adoption
---

There are two primary metrics that we recommend to track adoption of Sorbet in a
codebase:

1.  File-level typedness: the number of files at each
    [strictness level](static.md), like `# typed: false` or `# typed: true`.

2.  Callsite-level typedness: the number of method call sites where Sorbet knew
    whether the method existed or not. (Only tracked in `# typed: true` files or
    higher.)

Together, these two metrics provide a counter-balancing force for meaningfully
driving adoption of Sorbet in a codebase. In this doc, we'll cover:

- How to get Sorbet to report these metrics
- Why these metrics are good metrics
- Some suggestions for how to drive these metrics up

## Collecting metrics from Sorbet

Sorbet can collect metrics about a project while it's typechecking, and output
them at the end of the run. For example:

```bash
# The most basic invocation:
❯ srb tc --metrics-file=metrics.json

# A more complicated invocation:
❯ srb tc \
    --metrics-file="metrics.json" \
    --metrics-prefix="my_project" \
    --metrics-repo="my_org/my_project" \
    --metrics-branch="$(git rev-parse --abbrev-ref HEAD)" \
    --metrics-sha="$(srb --version | cut -d' ' -f 3)"
```

This will instruct Sorbet to typecheck the current project, and write out
various metrics into a file called `metrics.json` in the current folder.

The output looks something like this:

```js
{
 "repo": "my_project",
 "sha": "0.5.5278",
 "status": "Success",
 "branch": "master",
 "timestamp": "1581018717",
 "uuid": "0x180096dcfb381440-0x18c753ca-0x4074-0x8a2f-0xd1f9c82443c24b10x41a7",
 "metrics": [
  {
   "name": "my_project.run.utilization.user_time.us",
   "value": 37863
  },
  // ...
 ]
}
```

We'll cover which individual metrics to look at in the `"metrics"` array in the
next section. In the mean time, here are some config flags that affect how
metrics are reported:

- `--metrics-file=<file>.json`

  Instructs Sorbet to output metrics to a file. Without this flag, metrics are
  not recorded.

- `--metrics-repo=<repo>`

  Populates the `"repo"` field in the metrics output. Useful when using Sorbet
  on multiple repos.

- `--metrics-branch=<branch>`

  Populates the `"branch"` field in the metrics output. Useful for recording
  whether Sorbet was run on a `master` branch or a feature branch.

- `--metrics-prefix=<prefix>`

  Prepends the specified `<prefix>` in front of the name of each metric. Useful
  for enforcing various organization-specific naming conventions for metrics.

- `--metrics-sha=<sha>`

  Populates the `"sha"` field in the metrics output. (Stripe builds Sorbet from
  source in CI, and uses this field to track the commit SHA used when Sorbet was
  built.) Feel free to use it for whatever. In the example above, we use it to
  store the gem version of Sorbet that collected the metrics.

> **Note**: This list might be out of date. Be sure to check `srb tc --help`.

## Reporting metrics directly to statsd

In addition to outputing metrics to a file, Sorbet can also report metrics by
connecting directly to a statsd service. This technique is more advanced, which
means the instructions will heavily vary by organization and use case. At a high
level:

```bash
# Basic usage:
❯ srb tc --statsd-host="127.0.0.1"

# Advanced usage:
❯ srb tc \
    --statsd-host="127.0.0.1" \
    --statsd-port="9090" \
    --statsd-prefix="my_project"
```

This instructs Sorbet to typecheck the project and afterwards connect to a
statsd instance to report metrics about the runtime. What each option means:

- `--statsd-host=<host>`

  Connect to this statsd instance and report counters after Sorbet has finished.

- `--statsd-port=<port>`

  Connect to this port. Defaults to `8200`

- `--statsd-prefix=<prefix>`

  Prepends the specified `<prefix>` in front of the name of each metric. Useful
  for enforcing various organization-specific naming conventions for metrics.

## Which metrics to track

Sorbet reports many metrics. Here are the ones we recommend paying special
attention to for the purpose of tracking adoption of typedness:

### File-level typedness

```text
types.input.files
```

This metric counts the total number of files Sorbet ran over, including
[RBI files](rbi.md).

```text
types.input.files.sigil.ignore
types.input.files.sigil.false
types.input.files.sigil.true
types.input.files.sigil.strict
types.input.files.sigil.strong
```

These metrics count the number of files at each [strictness level](static.md)
within a project, including RBIs. For example, if a project had three files at
`# typed: true`, then sorbet would report `3` for
`types.input.files.sigil.true`.

### Call-site-level typedness

```text
types.input.sends.total
types.input.sends.typed
```

These metrics count the number of various method call sites ("sends") in a
codebase. There are two metrics here:

- `types.input.sends.total` is the total number of sends that are in
  `# typed: true` files or higher. Sorbet doesn't even attempt to type check
  method calls in `# typed: false` files, so this metric represents all the
  method call sites Sorbet looked at.

- `types.input.sends.typed` is the number of sends which Sorbet looked at
  **and** was sure that method existed. For example, in `x.foo`, if `x` is
  untyped, Sorbet doesn't know whether a method called `foo` exists. But if `x`
  is typed, Sorbet knows whether `foo` exists, how many arguments it takes, and
  what their types are.

  This metric is **not** the number of method call sites to methods with a
  `sig`. It's just the number of call sites where the receiver (`x` in `x.foo`)
  is not [untyped](untyped.md).

## Metrics philosophy

Why track these metrics? These metrics have been the primary metrics tracked by
numerous companies with successful track records of rolling out Sorbet to an
existing codebase.

Tracking file-level typedness makes sense. The [type sigil](static.md) in a file
controls whether type errors in that file are reported or silenced. Higher
strictness levels report more errors, so having more files in higher strictness
levels means that when Sorbet says "No errors! Great job." it carries more
weight.

Tracking call-site-level typedness usually makes less sense to people. For most
people, their gut instinct is to track "how many method definitons have
[signatures](sigs.md)." (That is, instead of tracking at the call site, people
are tempted to track at the definition site.)

Tracking call-site-level typedness is better for a number of reasons:

- Code breaks when it runs, not when it's defined. Call sites represent the
  individual places where code runs and thus could break. Call-site-level
  typedness informs how saturated the types check code that actually runs.

- [Untypedness is viral.](gradual.md) If one local variable is untyped, all
  calls on that method will return `T.untyped`. That result might be stored in a
  variable, and the process repeats. Having one source of untypedness in a
  method body can easily erase type information throughout the rest of the
  method body.

Thus, call-site-level typedness matches up more closely with people's intuitive
notion of "type coverage."

Call-site-level typedness is also a useful **counter-balance** to file-level
typedness. It's "easy" to make a `# typed: false` into a `# typed: true` file:
just use [`T.unsafe(...)`](troubleshooting.md#escape-hatches) to make individual
parts of that file untyped—in effect, it's the same as silencing errors with
`# typed: false`.

By tracking call-site-level typedness, using `T.unsafe` to work around problems
with `# typed: true` makes file-level typedness go up at the cost of
call-site-level typedness. This makes it harder to "game the system" by
increasing type coverage in a not-very-useful way.

## Suggestions for driving adoption

These two metrics (file-level and call-site-level typedness) are important at
different phases of adopting Sorbet in a codebase.

1.  During the ramp-up, file-level typedness is more important. While ramping
    up, it's important to enable the scope of Sorbet. For example, get as few
    files to be `# typed: ignore` as possible, then start moving towards making
    as many files `# typed: true` as possible. At this phase, **don't worry**
    too much about adding method signatures.

    This step usually involves resolving syntax errors or basic type errors in
    `ignored` or `false` files (like whether a constant is defined or not). This
    process helps to identify issues that make a codebase hard to type check.

2.  Once the majority of files are typed `# typed: true` or higher, it becomes
    important to drive up call-site-level typedness. This is a great time to
    figure out which are the most impactful files and methods to add type
    coverage to, and tackle those parts of a codebase first. These files might
    be files that are edited frequently, or where correctness and reliability
    are paramount. You can also [ask Sorbet what it thinks] are the most
    impactful methods.

    At this point, it's great to start adding signatures, as individual type
    annotations will help propagate type information further throughout a
    codebase. For example, find the core abstractions in a codebase (like the
    models) and add types to those first.

3.  When a codebase gets to a certain level of Sorbet maturity, it can be useful
    to require that new files be written at a certain strictness level. For
    example, files at `# typed: strict` are required to have signatures for all
    method definitions in that file. The [`rubocop-sorbet`] gem has a number of
    Rubocop rules for enforcing various Sorbet conventions.

4.  Rarely, in codebases that have managed to make heavy use of
    `# typed: strict`, it can make sense to use `# typed: strong`. At this
    level, Sorbet prevents using `T.untyped` in the file. We recommend that this
    level be used sparingly, only in files where 100% type coverage is essential
    (for example, possibly in the most error-prone parts of a codebase). By no
    means does it have to be a long term goal to achieve a high degree of
    `# typed: strong` files in a codebase. For example, Stripe's codebase of
    [over 150,000 files](https://stripe.com/blog/sorbet-stripes-type-checker-for-ruby)
    has only a couple dozen `# typed: strong` files.

[ask sorbet what it thinks]:
  https://github.com/sorbet/sorbet/blob/master/docs/suggest-sig.md
[`rubocop-sorbet`]: https://github.com/Shopify/rubocop-sorbet

## What next?

If you're curious to hear more about how other companies have approached
adopting Sorbet, here are some videos you can check out:

- [Adopting Sorbet at Scale], by Ufuk Kayserilioglu

  This talk explains how Shopify adopted Sorbet to leverage static typing in
  their Ruby codebase. They talk about their journey, the challenges they faced,
  and how they overcame them.

- [Type Checking Ruby on Rails Code], by Harry Doan

  This talk explains how the Chan-Zuckerberg Initiative developed and open
  sourced the `sorbet-rails` gem to make adopting Sorbet in a Rails codebase
  easier. They talk about specific challenges, and specific features of
  `sorbet-rails` that aided adoption in their codebase.

[adopting sorbet at scale]: https://www.youtube.com/watch?v=v9oYeSZGkUw
[type checking ruby on rails code]:
  https://chanzuckerberg.wistia.com/medias/mypzu8ie86
