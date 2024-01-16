---
id: metrics
title: Tracking Adoption with Metrics
sidebar_label: Tracking Adoption
---

There are two primary metrics that we recommend to track adoption of Sorbet in a
codebase:

1.  File-level typedness: the number of files at each
    [strictness level](static.md), like `# typed: false` or `# typed: true`.

2.  Usages of untyped: the number of times that something only typechecked
    because a piece of code relied on `T.untyped`. (Only tracked in
    `# typed: true` files or higher.)

> **Note**: in the past we recommended a different metric: callsite-level
> typedness. Sorbet still computes this metric, but it's not as useful as the
> new "usages of untyped" metric.

Together, these two metrics provide a counter-balancing force for meaningfully
driving adoption of Sorbet in a codebase. In this doc, we'll cover:

- How to get Sorbet to report these metrics
- Why these metrics are good metrics
- Some suggestions for how to drive these metrics up

## Collecting metrics from Sorbet

Sorbet can collect metrics about a project while typechecking and output them at
the end of the run. For example:

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

In addition to outputting metrics to a file, Sorbet can also report metrics by
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

### Usages of untyped

```text
types.input.untyped.usages
```

This metric counts the number of usages of untyped in the whole codebase. A
"usage of untyped" is basically anything where the code only typechecks because
a particular expression was `T.untyped`. To be more specific, "usage of untyped"
errors are the errors that Sorbet reports in files marked
[`# typed: strong`](strong.md). For more information, see
[What counts as a usage of untyped?](strong.md#what-counts-as-a-usage-of-untyped).

Note that this metric counts a single number: the total usages of untyped in the
entire codebase. For more granular untyped usage data, see the sections below.

### Call-site-level typedness

> **Note**: This metric is not as good as the usages of untyped metric.
>
> Sorbet still computes and reports it, but it won't track untyped as precisely
> as the `types.input.untyped.usages` metric above.

```text
types.input.sends.total
types.input.sends.typed
```

These metrics count the number of various method call sites ("sends") in a
codebase. There are two metrics here:

- `types.input.sends.total` is the total number of sends that are in
  `# typed: true` files or higher. Sorbet doesn't even attempt to type check
  individual method calls in `# typed: false` files, so this metric represents
  all the method call sites Sorbet looked at.

- `types.input.sends.typed` is the number of sends which Sorbet looked at
  **and** was sure that method existed. For example, in `x.foo`, if `x` is
  untyped, Sorbet doesn't know whether a method called `foo` exists. But if `x`
  is typed, Sorbet knows whether `foo` exists, how many arguments it takes, and
  what their types are.

  This metric is **not** the number of method call sites to methods with a
  `sig`. It's just the number of call sites where the receiver (`x` in `x.foo`)
  is not [untyped](untyped.md).

## Fine-grained untyped tracking

All the metrics reported with the `--metrics-file` or `--statsd` command are
coarse grained--they aggregate information across the entire codebase into a
single number.

Sorbet can compute more granular measurements about typing adoption in a
codebase, but it does not report these granular measurements via the
`--metrics-file`/`--statsd` mechanism.

Instead, Sorbet uses its `--print` option for more granular stats about typing
adoption, which instructs Sorbet to dump various internal data structures.

### Per-file usages of untyped

To report usages of untyped at a per-file level, pass these two flags to the
`srb tc` command:

```bash
srb tc --track-untyped --print=file-table-json:/tmp/file-table.json

# or, to output to stdout instead of a file:
srb tc --track-untyped --print=file-table-json
```

This instructs Sorbet to dump information about the number of untyped usages in
each file. The format looks something like this:

```javascript
{
 "files": [
  {
   "path": "foo/bar.rb",
   // ...
   "untyped_usages": 23
  },
  // ...
 ]
}
```

The full format for this output can be found in
[proto/File.proto](https://github.com/sorbet/sorbet/blob/master/proto/File.proto).
Note that due to idiosyncrasies in Sorbet's choice of serialization format (JSON
via Protobuf), keys whose values equal their "zero" value are omitted. This
means that a file which has no usages of untyped will simply lack an
`"untyped_usages"` key entirely.

Like with the `types.input.untyped.usages` metric, `# typed: false` and
`# typed: ignore` files are not considered for the sake of untyped usages. So
entries for these files will also lack an `"untyped_usages"` key.

### Blaming usages of untyped back to methods

Sorbet can attribute usages of untyped to the original source of untyped. The
docs for doing this live in the Sorbet source repo, because it involves a
slightly more involved process than other metrics:

→
[Blaming usages of untyped to definitions](https://github.com/sorbet/sorbet/blob/master/docs/untyped-blame.md)

To show how it works, consider an example like this:

```ruby
def method_without_a_sig_1 = 0
def method_without_a_sig_2 = 0

x = method_without_a_sig_1
x.even?

y = method_without_a_sig_2
y.even?
y.even?
```

In this snippet, there are three usages of untyped: one from a call to `.even?`
on `x` and two from a call to `.even?` on `y`. Sorbet can blame usages of
`T.untyped` back to its source, so that we could learn that this codebase has
one usage of untyped that blames to `method_without_a_sig_1`, and two that blame
to `method_without_a_sig_2`.

This is useful, because it essentially creates a punchlist of the methods that
have the best return on investment for adding a signature.

Unfortunately, tracking this information forces Sorbet change the memory layout
of certain internal data structures, which substantially increases the amount of
memory it uses when type checking. As such, blaming usages of untyped back to a
method requires building Sorbet from source. See
[Blaming usages of untyped to definitions](https://github.com/sorbet/sorbet/blob/master/docs/untyped-blame.md)
for more.

<br>

<br>

## Metrics philosophy

Why track these metrics? These metrics have been the primary metrics tracked by
numerous companies with successful track records of rolling out Sorbet to an
existing codebase.

Tracking file-level typedness makes sense. The [type sigil](static.md) in a file
controls whether type errors in that file are reported or silenced. Higher
strictness levels report more errors, so having more files in higher strictness
levels means that when Sorbet says "No errors! Great job." it carries more
weight.

Tracking usages of untyped usually makes less sense to people. For most people,
their gut instinct is to track "how many method definitions have
[signatures](sigs.md)." (That is, instead of tracking at the usage site, track
at the definition site.)

Tracking usages of untyped is better for a number of reasons:

- Code breaks when it runs, not when it's defined. Usages of untyped represent
  the individual places where code runs and thus could break.

- [Untypedness is viral.](gradual.md) If one local variable is initialized by a
  method that returns `T.untyped`, all calls on that method will return
  `T.untyped`. That result might be stored in a variable, and the process
  repeats. Having one source of untyped early in a method body can easily erase
  type information throughout the entire the method body.

Tracking usages of untyped matches up more closely with people's intuitive
notion of "type coverage."

Usages of untyped is also a useful **counter-balancing** metric to file-level
typedness. It's "easy" to make a `# typed: false` into a `# typed: true` file:
just use [`T.unsafe(...)`](troubleshooting.md#escape-hatches) to make individual
parts of that file untyped—in effect, it's the same as silencing errors with
`# typed: false`.

Using `T.unsafe` to work around problems with `# typed: true` makes file-level
typedness go up at the cost of making more usages of untyped. This makes it
harder to "game the system" by increasing type coverage in a not-very-useful
way.

## Suggestions for driving adoption

These two metrics (file-level typedness and usages of untyped) are important at
different phases of adopting Sorbet in a codebase.

1.  During the ramp-up, file-level typedness is more important. While ramping
    up, it's important to widen the scope of Sorbet in a codebase. For example,
    get as few files to be `# typed: ignore` as possible, then start moving
    towards making as many files `# typed: true` as possible. At this phase,
    **don't worry** too much about adding method signatures.

    This step usually involves resolving syntax errors or basic type errors in
    `ignored` or `false` files (like whether a constant is defined or not). This
    process helps to identify issues that make a codebase hard to type check.

2.  Once the majority of files are typed `# typed: true` or higher, it becomes
    important to drive down usages of untyped. This is a great time to figure
    out which are the most impactful files and methods to add type coverage to,
    and tackle those parts of a codebase first. These files might be files that
    are edited frequently, or where correctness and reliability are paramount.
    You can [ask Sorbet what it thinks] are the most impactful methods.

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
  https://github.com/sorbet/sorbet/blob/master/docs/untyped-blame.md
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
