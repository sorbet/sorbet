# Blaming usages of untyped to definitions

It's possible to build Sorbet in a way that it can "blame" usages of untyped
back to a definition (like a method or instance variable) which introduced the
usage of untyped in the first place. With this information, it becomes easier to
evaluate the cost vs benefit of improving the type annotations on any given
method.

It requires building Sorbet with a special build configuration, because simply
adding the capability to blame untyped usages to definitions (regardless of
whether the user actually cares to use that capability) causes Sorbet to use
substantially more memory for certain internal data structures.

Once built, it works something like this. Given an input file like this:

```ruby
# typed: true

def method_without_a_sig_1 = 0
def method_without_a_sig_2 = 0

x = method_without_a_sig_1
x.even?
x.even?

y = method_without_a_sig_2
y.even?
```

Sorbet can produce untyped blame output like this:

```
❯ sorbet --track-untyped --print=untyped-blame foo.rb
No errors! Great job.
[
  {
    "path": "foo.rb",
    "package": "<none>",
    "owner": "Object",
    "name": "method_without_a_sig_2",
    "count": 1
  },
  {
    "path": "foo.rb",
    "package": "<none>",
    "owner": "Object",
    "name": "method_without_a_sig_1",
    "count": 2
  }
]
```

## Building Sorbet with untyped blame support

1.  Follow the [Quickstart] instructions in Sorbet's README.

    These instructions will ensure that have a working build environment. If you
    encounter problems attempting to build Sorbet, feel free to ask for help in
    the #internals channel on the [Sorbet Slack](https://sorbet.org/slack).

1.  Build a release version of Sorbet with untyped blame support. The
    instructions differ by platform.

    For Linux:

    ```
    ./bazel build //main:sorbet --config=release-linux --config=untyped-blame
    ```

    For macOS:

    ```
    ./bazel build //main:sorbet --config=release-mac --config=untyped-blame
    ```

1.  Test that the new build works:

    ```
    ❯ bazel-bin/main/sorbet --track-untyped --print=untyped-blame -e 'T.unsafe(nil).foo'
    No errors! Great job.
    [{"path":"https://github.com/sorbet/sorbet/tree/master/rbi/sorbet/t.rbi","package":"<none>","owner":"T.class_of(T)","name":"unsafe","count":1}]
    ```

[Quickstart]: https://github.com/sorbet/sorbet/#quickstart

## Collecting untyped blame information

Sorbet collects untyped blame information by type checking a project, and
dumping JSON information about untyped usages at the end. To use this new build
of Sorbet to type check a project, change into your project's directory and run
this command:

```bash
SRB_SORBET_EXE=path/to/bazel-bin/main/sorbet bundle exec srb tc \
  --track-untyped --print=untyped-blame
```

**Note**: Replace `path/to/...` with the real path to the newly-built sorbet
binary.

The untyped blame information will output to stdout. To redirect it to a file,
either use a Unix pipe, or use this form of the `--print` option, which will put
the print output directly into a file (in this case, it will create the
`/tmp/untyped-blame.json` file):

```bash
SRB_SORBET_EXE=path/to/bazel-bin/main/sorbet bundle exec srb tc \
  --track-untyped --print=untyped-blame:/tmp/untyped-blame.json
```

## Interpreting the output

The output will be a JSON array of JSON objects, where each object represents a
definition, alongside how many usages of untyped blame to that definition.

Each entry has these keys:

- `owner`

  The fully qualified name of the owner of the definition. For a method like
  `A::B#foo`, the owner will be `A::B`. For a method like `A::B.foo`, the owner
  will be `T.class_of(A::B)`.

- `name`

  The name of the definition. For a method like `A::B#foo`, the name will be
  `foo`. The name will also be `foo` for a method like `A::B.foo` (though the
  `owner` will be different, see above).

- `path`

  The path to the primary location of the definition. The primary location of
  the definition is a Sorbet-internal heuristic that takes into account whether
  the location is in an RBI file or not as well as the strictness level of that
  file.

- `count`

  The number of usages of untyped that blame to this definition.

- `package`

  Will always be `<none>`, unless using Stripe's internal Ruby packaging
  system. In that case, `package` will be the name of the package that owns this
  definition.


### Some special definitions

There are some special definitions in the output that don't map to definitions
defined anywhere in code.

- `owner = <Magic>::<UntypedSource>`

  Some usages of untyped arise because of `T.untyped` that arises internal to
  Sorbet. For example, most existing limitation with [shape
  types](https://sorbet.org/docs/shapes) arise because Sorbet will silently
  introduce a usage of untyped.

  In these cases, there is not a specific definition Sorbet can blame the
  untyped to, so Sorbet invents some synthetic definitions to attribute the
  untyped.

  All these symbols live under the special owner `<Magic>::<UntypedSource>`.
  Some examples:

  - `name = <super>`

    Sorbet is not always able to infer a type for a call to `super`. When that
    happens, Sorbet implicitly treats the `super` as a method that returns
    untyped, and blames the untyped to this synthetic definition.

    Read more: [Why is `super` untyped?](https://sorbet.org/docs/faq#why-is-super-untyped-even-when-the-parent-method-has-a-sig)

  - `name = <proc>`

    Sorbet does not infer types for `proc` and `lambda` literals the same way it
    does for usages of blocks attached to method calls. When creating a `proc`
    or `lambda` value, the inferred type will be a `T.proc` type where all the
    params and return type of the proc are `T.untyped`.

- `name = <undeclared-field-stub>`

  This represents usages of untyped in the program arising from undeclared
  instance variables.

  In files below `# typed: strict`, Sorbet does not require instance variables
  to be declared. As such, it is not always able to correlate a usage of an
  instance variable with a certain name to a definition. When this happens,
  Sorbet assumes that the instance variable corresponds to some hypothetical
  instance variable whose type is `T.untyped`. It then blames the usage of
  untyped to this synthetic `<Magic>::<undeclared-field-stub>` definition.

- `name = <none>`

  Not all usages of untyped blame to a definition. Ideally, every use
  of untyped would blame to either an internal, `<Magic>::<UntypedSource>`
  definition or to a definition in the codebase itself.

  But we have not audited all usages of untyped in Sorbet itself, so some usages
  of untyped have not yet been annotated with a suitable
  `<Magic>::<UntypedSource>` blame.

  If you're interested in helping out, feel free to search Sorbet's codebase for
  `Types::untypedUntracked` and convert them to usages of
  `Types::untyped(definition)`.


## Automatically suggesting method signatures

Having discovered which methods are the most important to typecheck, you may
want to attempt to add signatures to methods in bulk to drive down usage of
untyped.

Sorbet has multiple tools for this, documented on
[sorbet.org](https://sorbet.org):

→ [Automatically suggesting method signatures](https://sorbet.org/docs/sig-suggestion)

