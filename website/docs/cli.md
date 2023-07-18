---
id: cli
title: Command Line Reference
---

Sorbet has a wealth of command line options; we'll only be covering a subset of
them here. For the full set of available options, check the help at the command
line:

```bash
# to learn about available srb subcommands
❯ srb --help

# for options that Sorbet uses when typechecking
❯ srb tc --help
```

In this doc, we'll cover **only** the command line interface to the `srb tc`
subcommand. For information about `tapioca` and `srb rbi`, see
[RBI files](rbi.md).

## Config file

The first time `tapioca init` runs, it creates a config file that `srb tc` will
read, located at `sorbet/config`. The config file is actually just a
newline-separated list of arguments to pass to `srb tc`, the same as if they'd
been passed at the command line.

```plain
--dir
.
# Full-line comment
--ignore
path/to/ignore.rb
```

Every line in this file acts as if it's prepended to the argument list of
`srb tc`. So arguments in the config file are always passed first (if it
exists), followed by arguments provided on the command line.

For a full description of the config file format, see the output of
`srb tc --help`.

To skip loading the config file (i.e., to create minimal repro examples to
report an issue), there's the `--no-config` flag:

```
srb tc --no-config ...
```

This makes it so that the only args Sorbet reads are those explicitly passed on
the command line.

Next we'll call out some specific flags that are likely to help troubleshoot
problems or adapt Sorbet into an existing project.

## Including and excluding files

First, some examples:

```bash
# All *.rb / *.rbi files in the current folder
srb tc .

# Only the file foo.rb (+ any paths mentioned in sorbet/config)
srb tc foo.rb

# All *.rb / *.rbi files in the current folder,
# minus any file in the vendor folder of the current directory.
srb tc . --ignore=/vendor
```

`srb tc` can be given a list of paths (either folder or files) to Ruby files
that it should read. By default, `sorbet/config` is created to contain `.`, so
`srb tc` will type check every file in the current directory[^gems].

> **Note**: Sorbet only checks files that end in `*.rb` or `*.rbi`. To check
> other files, they must be explicitly named on the command line (or in the
> config file), or given an appropriate file extension.

<!-- prettier-ignore-start -->

[^gems]: This is incidentally why Sorbet does not type check gems: the paths to
gems' source directories are never given to Sorbet.

<!-- prettier-ignore-end -->

Sometimes, including an entire folder includes too many files. We can refine the
list of files Sorbet reads with the `--ignore` flag. The syntax is:

```plaintext
--ignore <pattern>
```

This will ignore input files that contain the given pattern in their paths
(relative to the input path passed to Sorbet). Patterns beginning with / match
against the prefix of these relative paths; others are substring matches.
Matches must be against whole component parts, so `foo` matches `/foo/bar.rb`
and `/bar/foo/baz.rb` but not `/foo.rb` or `/foo2/bar.rb`.

## Accepting autocorrect suggestions

For certain errors, Sorbet suggests autocorrects that can be accepted to
automatically fix those errors. For example:

```ruby
sig {params(xs: T::Array[Integer]).void}
def example(xs)
  xs[0] + 1 # error: Method `+` does not exist on `NilClass` component
end

example([1, 2, 3])
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28xs%3A%20T%3A%3AArray%5BInteger%5D%29.void%7D%0Adef%20example%28xs%29%0A%20%20xs%5B0%5D%20%2B%201%0Aend%0A%0Aexample%28%5B1%2C%202%2C%203%5D%29)

In this example, Sorbet does not think that the `+` method always exists,
because `xs[0]` might be `nil` if the `xs` list is empty.

In this example, though, we know that the `xs` array will always be non-empty.
If, as the programmer, we know this invariant will always hold, we can use
`T.must` to fix the problem. Sorbet even suggests that in the error message:

```
editor.rb:6: Method `+` does not exist on `NilClass` component of `T.nilable(Integer)` https://srb.help/7003
     6 |  xs[0] + 1
                ^
  Got `T.nilable(Integer)` originating from:
    editor.rb:6:
     6 |  xs[0] + 1
          ^^^^^
  Autocorrect: Use `-a` to autocorrect
    editor.rb:6: Replace with `T.must(xs[0])`
     6 |  xs[0] + 1
          ^^^^^
Errors: 1
```

To accept this suggestion, we can re-run Sorbet with the `-a` or `--autocorrect`
command line flag:

```
❯ srb tc --autocorrect
```

### Limiting autocorrect suggestions

By default, Sorbet will apply **all** autocorrect suggestions when the `-a` or
`--autocorrect` flag is provided. To only have Sorbet apply some autocorrect
suggestions, use the `--isolate-error-code` flag with a Sorbet error code. For
example, using the error code in the above error message:

```
❯ srb tc --autocorrect --isolate-error-code=7003
```

The `--isolate-error-code` option can be repeated with different error codes to
have Sorbet apply autocorrects for all the mentioned error codes, but no others.

There is no way to have Sorbet **only** apply autocorrect suggestions in a given
file. Instead, use a version control system to undo autocorrect changes Sorbet
makes in files that should not be changed.

Sometimes, Sorbet autocorrects include suggestions to rename a method, for
example to fix a supposed typo. Sometimes these "did you mean" suggestions can
accidentally change the meaning of a program. To disable these did you mean
suggestions, use `--did-you-mean=false`:

```
❯ srb tc --autocorrect --did-you-mean=false
```

### Silencing errors in bulk

Sometimes, Sorbet doesn't know how to **fix** an error, it only knows how to
**silence** the error. Sorbet does not show these autocorrect suggestions by
default, because it does not want to train people, especially new Sorbet users,
to blindly silence type errors.

However, there are cases where silencing errors in bulk can be useful (e.g.,
when doing a large-but-incremental refactor to a core library, or upgrading to a
new Sorbet version that introduces many new type errors).

Use the `--suggest-unsafe` flag to have Sorbet additionally produce autocorrect
suggestions that will unconditionally silence certain errors. Use
`--suggest-unsafe --autocorrect` to accept these suggestions. For example, here
is a program that Sorbet would not usually suggest an autocorrect for:

```ruby
sig {params(x: T.any(Integer, String)).void}
def example(x)
  1 + x # error: Expected `Integer` but found `T.any(Integer, String)`
end

example(1)
```

But if we pass the `--suggest-unsafe` flag to Sorbet, it will suggest an
autocorrect:

```
❯ srb tc --suggest-unsafe
editor.rb:6: Expected `Integer` but found `T.any(Integer, String)` for argument `arg0` https://srb.help/7002
     6 |  1 + x
              ^
  Expected `Integer` for argument `arg0` of method `Integer#+`:
    https://github.com/sorbet/sorbet/tree/master/rbi/core/integer.rbi#L148:
     148 |        arg0: Integer,
                  ^^^^
  Got `T.any(Integer, String)` originating from:
    editor.rb:5:
     5 |def example(x)
                    ^
  Autocorrect: Use `-a` to autocorrect
    editor.rb:6: Replace with `T.unsafe(x)`
     6 |  1 + x
              ^
Errors: 1
```

[→ View without `--suggest-unsafe` on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28x%3A%20T.any%28Integer%2C%20String%29%29.void%7D%0Adef%20example%28x%29%0A%20%201%20%2B%20x%0Aend%0A%0Aexample%281%29)\
[→ View with `--suggest-unsafe` on sorbet.run](https://sorbet.run/?arg=--suggest-unsafe#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28x%3A%20T.any%28Integer%2C%20String%29%29.void%7D%0Adef%20example%28x%29%0A%20%201%20%2B%20x%0Aend%0A%0Aexample%281%29)

Note how the autocorrect suggestion here suggests adding `T.unsafe`, but only
when the `--suggest-unsafe` flag was provided.

We recommend using `--suggest-unsafe` especially when doing Sorbet version
upgrades. Usually the reason why a Sorbet version upgrade introduces a new error
is because some old behavior was silently hiding a type error that was **already
present** in the codebase. Using `T.unsafe` to silence the existing errors makes
it easy to quickly upgrade to the new version, effectively locking newly-written
code from suffering from that Sorbet bug. It's always possible to audit old
usages of `T.unsafe`. Sorbet gets better with each release, and upgrading Sorbet
early and often is usually less work over time.

Sorbet also allows customizing the unsafe suggestions: given
`--suggest-unsafe=<custom method>`, Sorbet will use the `<custom method>` in
place of `T.unsafe` in all suggestions. For example:

```
❯ srb tc --suggest-unsafe=XXX.sorbet_0_5_1234 --autocorrect
```

This would cause case Sorbet to suggest converting `x` in the example above to
`XXX.sorbet_0_5_1234(x)`. It would then be possible to define a module like
this:

```ruby
module XXX
  # Automatically-suppressed error during upgrade to Sorbet version 0.5.1234
  #
  # If you see this method used in a file your team owns, consider removing the
  # call and fixing the resulting error, or replacing it with `T.unsafe` if the
  # error cannot be fixed.
  def self.sorbet_0_5_1234(x); x; end
end
```

A method named like this sticks out in the codebase as a flag to potential
developers that something has been automatically silenced instead of being
manually silenced. Placing a comment on the method shows the contextual message
when people hover over any call site.

## Overriding strictness levels

By default, Sorbet reads each file to determine its
[strictness level](static.md#file-level-granularity-strictness-levels),
defaulting to `# typed: false` if none is provided.

It can be useful, especially when [Adopting Sorbet](adopting.md) to see what it
would be like if certain files were at a different strictness level. There are a
handful of flags that do this:

#### `--typed <level>`

This option forces every file Sorbet reads to `# typed: <level>`. For example:

```plaintext
❯ srb tc --typed=true
```

Will report errors in every file as if they'd been declared `# typed: true`.

#### `--typed-override filepath.yaml`

This option is similar, except we can provide a YAML file that declares
overrides on a file-by-file basis. For example, with this YAML file:

```yaml
# -- foo.yaml --
true:
  - ./foo.rb
```

and this Ruby file:

```ruby
# -- foo.rb --
# typed: false
"string" + :symbol
```

This will be the output using the `--typed-override` flag:

```plaintext
❯ srb tc --typed-override=foo.yaml foo.rb
foo.rb:3: Expected `String` but found `Symbol(:"symbol")` for argument `arg0`
...
```

## Is there a way to get errors in JSON format?

There is not, intentionally. If you're trying to consume Sorbet's output from a
script, we recommend you either consume Sorbet via the LSP protocol, or using
standard UNIX tools like `sed` and `awk` to extract information from the
human-readable output.

Why? Committing to a stable output format is an ongoing maintenance burden for
the Sorbet team. We have explicitly chosen only two output formats:

- The human-readable error output seen when running `srb tc` from the CLI
  directly.

- Output conforming to the [Language Server Protocol][lsp], which drives editor
  integrations (see the `--lsp` flag).

[lsp]: https://microsoft.github.io/language-server-protocol/

The LSP spec is stable, versioned, typed, and easy to test whether Sorbet
conforms to it. We don't want to be in the business of inventing competing
editor output formats, so we explicitly only support these two.
