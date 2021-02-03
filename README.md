<p align="center">
  <img alt="Sorbet logo" width="200" src="docs/logo/sorbet-logo-purple-sparkles.svg">
</p>

# Sorbet

This repository contains Sorbet, a fast, powerful type checker designed for Ruby.
It aims to be easy to add to existing codebases with gradual types, and fast to
respond with errors and suggestions.

This README contains documentation specifically for contributing to Sorbet. You
might also want to:

- Read the [public Sorbet docs](https://sorbet.org/docs/overview)
  - Or even [edit the docs](#writing-docs)
- Watch the [talks we've given](https://sorbet.org/en/community#talks) about Sorbet
- Try the [Sorbet playground](https://sorbet.run) online

If you are at Stripe, you might also want to see <http://go/types/internals> for
docs about Stripe-specific development workflows and historical Stripe context.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Table of Contents

- [Sorbet user-facing design principles](#sorbet-user-facing-design-principles)
- [Quickstart](#quickstart)
- [Learning how Sorbet works](#learning-how-sorbet-works)
- [Building Sorbet](#building-sorbet)
  - [Common Compilation Errors](#common-compilation-errors)
- [Running Sorbet](#running-sorbet)
- [Running the tests](#running-the-tests)
- [Testing Sorbet against pay-server](#testing-sorbet-against-pay-server)
- [Writing tests](#writing-tests)
  - [test_corpus tests](#test_corpus-tests)
  - [Expectation tests](#expectation-tests)
  - [CLI tests](#cli-tests)
  - [LSP tests](#lsp-tests)
    - [Testing "Find Definition" and "Find All References"](#testing-find-definition-and-find-all-references)
    - [Testing "Go to Type Definition"](#testing-go-to-type-definition)
    - [Testing hover](#testing-hover)
    - [Testing completion](#testing-completion)
    - [Testing workspace symbols (symbol search)](#testing-workspace-symbols-symbol-search)
    - [Testing incremental type checking](#testing-incremental-type-checking)
  - [LSP recorded tests](#lsp-recorded-tests)
  - [Updating tests](#updating-tests)
- [Debugging](#debugging)
- [Writing docs](#writing-docs)
- [Editor and environment](#editor-and-environment)
  - [Bazel](#bazel)
  - [Multiple git worktrees](#multiple-git-worktrees)
  - [Shell](#shell)
  - [Formatting files](#formatting-files)
  - [Editor setup for C++](#editor-setup-for-c)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Sorbet user-facing design principles

Early in our project, we've defined some guidelines for how working with sorbet should feel like.

1. **Explicit**

    We're willing to write annotations, and in fact see them as
    beneficial; they make code more readable and predictable. We're here
    to help readers as much as writers.

2. **Feel useful, not burdensome**

    While it is explicit, we are putting effort into making it concise.
    This shows in multiple ways:
     - error messages should be clear
     - verbosity should be compensated with more safety

3. **As simple as possible, but powerful enough**

    Overall, we are not strong believers in super-complex type
    systems. They have their place, and we need a fair amount of
    expressive power to model (enough) real Ruby code, but all else
    being equal we want to be simpler. We believe that such a system
    scales better, and—most importantly—is easier for our users to
    learn & understand.

4. **Compatible with Ruby**

    In particular, we don't want a new syntax. Existing Ruby syntax means
    we can leverage most of our existing tooling (editors, etc). Also,
    the point of Sorbet is to gradually improve an existing Ruby codebase. No
    new syntax makes it easier to be compatible with existing tools.

5. **Scales**

    On all axes: execution speed, number of collaborators, lines of code,
    codebase age. We work in large Ruby codebases, and they will only get
    larger.

6. **Can be adopted gradually**

    In order to make adoption possible at scale, we cannot require every team or
    project to adopt Sorbet all at once. Sorbet needs to support teams adopting
    it at different paces.

## Quickstart

1.  Install the dependencies

    - `brew install bazel autoconf coreutils parallel`

2.  Clone this repository

    - `git clone https://github.com/sorbet/sorbet.git`
    - `cd sorbet`

3.  Build Sorbet

    - `./bazel build //main:sorbet --config=dbg`

4.  Run Sorbet!

    - `bazel-bin/main/sorbet -e "42 + 'hello'"`


## Learning how Sorbet works

We've documented the [internals of Sorbet](docs/internals.md) in a separate doc.
Cross-reference between that doc and here to learn how Sorbet works and how to
change it!

[→ internals.md](docs/internals.md)

There is also a talk online that describes Sorbet's high-level architecture and
the reasons why it's fast:

[→ Fast type checking for Ruby](https://sorbet.org/docs/talks/jvm-ls-2019)


## Building Sorbet

There are multiple ways to build `sorbet`. This one is the most common:

```
./bazel build //main:sorbet --config=dbg
```

This will build an executable in `bazel-bin/main/sorbet` (see "Running Sorbet"
below). There are many options you can pass when building `sorbet`:

- `--config=dbg`
  - Most common build config for development.
  - Good stack traces, runs all ENFORCEs.
- `--config=sanitize`
  - Link in extra sanitizers, in particular: UBSan and ASan.
  - Catches most memory and undefined-behavior errors.
  - Substantially larger and slower binary.
- `--config=debugsymbols`
  - (Included by `--config=dbg`) debugging symbols, and nothing else.
- `--config=forcedebug`
  - Use more memory, but report even more sanity checks.
- `--config=static-libs`
  - Forcibly use static linking (Sorbet defaults to dynamic linking for faster
    build times).
  - Sorbet already uses this option in release builds (see below).
- `--config=release-mac` and `--config=release-linux`
  - Exact release configuration that we ship to our users.

Independently of providing or omitting any of the above flags, you can turn on
optimizations for any build:

- `-c opt`
  - Enables `clang` optimizations (i.e., `-O2`)

These args are not mutually exclusive. For example, a common pairing when
debugging is

```
--config=dbg --config=sanitize
```

In `.bazelrc` you can find out what all these options (and others) mean.

### Common Compilation Errors

**(Mac) `Xcode version must be specified to use an Apple CROSSTOOL`**

This error typically occurs after an Xcode upgrade.

Developer tools must be installed, the Xcode license must be accepted, and
your active Xcode command line tools directory must point to an installed
version of Xcode.

The following commands should do the trick:

```shell
# Install command line tools
xcode-select --install
# Ensure that the system finds command line tools in an active Xcode directory
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
# Accept the Xcode license.
sudo xcodebuild -license
# Clear bazel's cache, which may contain files generated from a previous
# version of Xcode command line tools.
bazel clean --expunge
```

**(Mac) `fatal error: 'math.h' file not found`** (or some other system header)

This error can happen on Macs when the `/usr/include` folder is missing. The
solution is to install macOS headers via the following package:

macOS Mojave:
```shell
open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg
```

macOS Catalina:
```shell
sudo ln -s /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/* /usr/local/include/
```

## Running Sorbet

Run Sorbet on an expression:

```
bazel-bin/main/sorbet -e "1 + false"
```

Run Sorbet on a file:

```
bazel-bin/main/sorbet foo.rb
```

Running `bazel-bin/main/sorbet --help` will show lots of options. These are
the common ones for contributors:


- `-p <IR>`
  - Asks sorbet to print out any given intermediate representation.
  - See `--help` for available values of `<IR>`.
- `--stop-after <phase>`
  - Useful when there's a bug in a later phase, and you want to quit early to
    debug.
- `-v`, `-vv`, `-vvv`
  - Show `logger` output (increasing verbosity)
- `--max-threads=1`
  - Useful for determining if you're dealing with a concurrency bug or not.
- `--wait-for-dbg`
  - Will freeze Sorbet on startup and wait for a debugger to attach
  - This is useful when you don't have control over launching the process (LSP)


## Running the tests

To run all the tests:

```
bazel test //... --config=dbg
```

(The `//...` literally means "all targets".)

To run a subset of the tests curated for faster iteration and development speed,
run:

```
bazel test test --config=dbg
```

Note that in bazel terms, the second test is an alias for `//test:test`, so we're being a bit cute here.

By default, all test output goes into files. To also print it to the screen:

```
bazel test //... --config=dbg --test_output=errors
```

If any test failed, you will see two pieces of information printed:

```
1. //test:test_testdata/resolver/optional_constant
2.   /private/var/tmp/.../test/test_testdata/resolver/optional_constant/test.log
```

1.  the test's target (in case you want to run just this test again with `bazel
    test <target>`)
2.  a (runnable) file containing the test's output

To see the failing output, either:

- Re-run `bazel test` with the `--test_output=errors` flag
- Copy/paste the `*.log` file and run it (the output will open in `less`)


## Testing Sorbet against pay-server

> This is specific to contributing to Sorbet at Stripe.

If you are at Stripe and want to test your branch against pay-server, see
<http://go/types/local-dev>.

## Writing tests

We write tests by adding files to subfolders of the `test/` directory.
Individual subfolders are "magic"; each contains specific types of tests.
We aspire to have our tests be fully reproducible.

> **C++ note**: In C++, hash functions are only required to produce the same
> result for the same input within a single execution of a program.
>
> Thus, we expect all user-visible outputs to be explicitly sorted using a
> key stable from one run to the next.

There are many ways to test Sorbet, some "better" than others. We've ordered
them below in order from most preferable to least preferable. And we always
prefer some tests to no tests!

### test_corpus tests

The first kind of test can be called either [test_corpus] tests or [testdata]
tests, based on the name of the test harness or the folder containing these tests, respectively.


[test_corpus]: test/pipeline_test_runner.cc
[testdata]: test/testdata/

To create a test_corpus test, add any file `<name>.rb` to `test/testdata`, in
any folder depth. The file must either:

- type check entirely, or
- throw errors **only** on lines marked with a comment (see below).

To mark that a line should have errors, append `# error: <message>` (the
`<message>` must match the raised error message). In case there are multiple
errors on this line, add an `# error: <message>` on its own line just below.

Error checks can optionally point to a range of characters rather than a line:

```ruby
1 + '' # error: `String` doesn't match `Integer`

rescue Foo, Bar => baz
     # ^^^ error: Unable to resolve constant `Foo`
          # ^^^ error: Unable to resolve constant `Bar`
```

You can run this test with:

```
bazel test //test:test_PosTests/testdata/path/to/<name>
```

### Expectation tests

Each test_corpus test can be turned into an expectation test by optionally
creating any number of `<name>.rb.<phase>.exp` files (where `<name>` matches the
name of the ruby file for this test). These files contain pretty printed
representations of internal data structures, according to what would be printed
by `-p <phase>`. The snapshot must exactly match the output generated by running
`sorbet -p <phase> <name>.rb` for the test to pass.

You can run this test with:

```
bazel test //test:test_PosTests/testdata/path/to/<name>
```

Files that begin with a prefix and `__` will be run together. For example,
`foo__1.rb` and `foo__2.rb` will be run together as test `foo`.

### CLI tests

Any folder `<name>` that is added to `test/cli/` becomes a test.
This folder should have a file `<name>.sh` that is executable.
When run, its output will be compared against `<name>.out` in that folder.

Our bazel setup will produce two targets:

- `bazel run //test/cli:test_<name>` will execute the `.sh` file
- `bazel test //test/cli:test_<name>` will execute the `.sh` and check it against what's in
  the `.out` file.

The scripts are run inside Bazel, so they will be executed from the top of the
workspace and have access to source files and built targets using their path
from the root. In particular, the compiled sorbet binary is available under
`main/sorbet`.

### LSP tests

Most LSP tests are expectation tests with additional LSP-specific annotations.
They are primarily contained in `test/testdata/lsp`, but all files in `test/testdata`
are tested in LSP mode. You can run a test `test/testdata/lsp/<name>.rb` like so:

```
bazel test //test:test_LSPTests/testdata/lsp/<name>
```

#### Testing "Find Definition" and "Find All References"

LSP tests have access to `def` and `usage` assertions that you can use to annotate definition
and usage sites for a variable:

```ruby
  a = 10
# ^ def: a
  b = a + 10
    # ^ usage: a
```

With these annotations, the test will check that "Find Definition" from the addition will lead to
`a = 10`, and that "Find All References" from either location will return both the definition and usage.

If a variable is re-defined, it can be annotated with a version number:

```ruby
  a = 10
# ^ def: a 1
  a = 20
# ^ def: a 2
  b = a + 10
    # ^ usage: a 2
```

`usage` annotations can accept multiple version numbers, separated by a `,`. This is useful if you have variables that
get re-defined through multiple-paths:

```ruby
  if some_condition
    a = 10
  # ^ def a 1
  else
    a = 'hello'
  # ^ def: a 2
  end

  p a
  # ^ usage: a 1,2
```

If a location should not report any definition or usage, then use the magic label `(nothing)`:

```ruby
    a = 10
# ^ def: (nothing)
```

If a location should report multiple definitions (e.g., a class or module opened
in multiple files), then you can add a second `def` with the same name:

```ruby
class Foo
  #   ^^^ def: foo
end

class Foo
  #   ^^^ def: foo
end
```

When marking definitions that correspond to method arguments that have defaults, multiple definitions will need to be
marked: one for the argument definition itself and one for its default value. The default value needs to be given a
different version number, and also marked `default-arg-value`:

```ruby
  def foo(a: 1)
        # ^ def: a 1
           # ^ def: a 2 default-arg-value
    p a
    # ^ usage: a 1,2
  end
```

This is due to the translation of defaults into the CFG: there is a synthetic conditional that chooses either to
initialize the variable from the argument passed at the send, or to the default value when no value is present.

#### Testing "Go to Type Definition"

This is somewhat similar to "Find Definition" above, but also slightly different
because there's no analogue of "Find All Type Definitions."

```ruby
class A; end
#     ^ type-def: some-label

aaa = A.new
# ^ type: some-label
```

The `type: some-label` assertion says "please simulate a Go to Type Definition
here, named `some-label`" and the `type-def: some-label` assertion says "assert
that the results for `some-label` are exactly these locations."

That means if the type definition could return multiple locs, the assertions
will have to cover all results:

```ruby
class A; end
#     ^ type-def: AorB
class B; end
#     ^ type-def: AorB

aaa = T.let(A.new, T.any(A, B))
# ^ type: AorB
```

If a location should not report any definition or usage, then use the magic
label `(nothing)`:

```ruby
# typed: false
class A; end
aaa = A.new
# ^ def: (nothing)
```

#### Testing hover

LSP tests can also assert the contents of hover responses with `hover` assertions:

```ruby
  a = 10
# ^ hover: Integer(10)
```

If a location should report the empty string, use the special label `(nothing)`:

```ruby
     a = 10
# ^ hover: (nothing)
```

#### Testing completion

LSP tests can also assert the contents of completion responses with `completion`
assertions.

```ruby
class A
  def self.foo_1; end
  def self.foo_2; end

  foo
#    ^ completion: foo_1, foo_2
end
```

The `^` corresponds to the position of the cursor. So in the above example, it's
as if the cursor is like this: `foo│`. If the `^` had been directly under the
last `o`, it would have been like this: `fo|o`. Only the first `^` is used. If
you use `^^^` in the assertion, the test harness will send a completion
assertion at the position of the first caret.

You can also write a test for a partial prefix of the completion results:

```ruby
class A
  def self.foo_1; end
  def self.foo_2; end

  foo
#    ^ completion: foo_1, ...
end
```

Add the `, ...` suffix to the end of a partial list of completion results, and
the test harness will ensure that the listed identifiers match a prefix of the
completion items. This prefix must still be listed in order.

If a location should report zero completion items, use the special message
`(nothing)`:

```ruby
class A
  def self.foo_1; end
  def self.foo_2; end

  zzz
#    ^ completion: (nothing)
end
```

To write a test for the snippet that would be inserted into the document if a
particular completion item was selected, you can make two files:

```
# -- test/testdata/lsp/completion/mytest.rb --
class A
  def self.foo_1; end
end

A.foo_
#     ^ apply-completion: [A] item: 0
```

The `apply-completion` assertion says "make sure the file `mytest.A.rbedited`
contains the result of inserting the completion snippet for the 0th completion
item into the file."

```
# -- test/testdata/lsp/completion/mytest.A.rbedited --
class A
  def self.foo_1; end
end

A.foo_1${0}
#     ^ apply-completion: [A] item: 0
```

As you can see, the fancy `${...}` (tabstop placeholders) show up verbatim in
the output if they were sent in the completion response.

It's not currently possible to test these parts of a completion response:

- completion kind
- documentation
- detail

For these, your best bet is to test manually in VS Code / your preferred editor
and verify that you're seeing your changes. For documentation specifically,
nearly all the code paths are shared with hover, so you can alternatively write
a hover test.

#### Testing workspace symbols (symbol search)

LSP tests can assert that a specific item appears in a symbol search (the
`textDocument/workspaceSymbols` request) using the `symbol-search` assertion:

```ruby
class Project::Foo
#     ^^^ symbol-search: "Foo"
end
```

The `symbol-search` can optionally specify _how_ that item should appear in
search results:

```ruby
class Project::Foo
#     ^^^ symbol-search: "Foo", name = "Foo", container = "Project"
end
```

In the above, `container` can also be the special string `"(nothing)"` to
indicate that the item has no container.

`symbol-search` can also specify the item's relative rank in the ordered
search results:

```ruby
class Project::Foo
#     ^^^ symbol-search: "Foo", rank = 1
end
```

#### Testing rename constant

To write a test for renaming constants, you need to make at least two files:

```ruby
# -- test/testdata/lsp/refactor/mytest.rb --

# typed: true
# frozen_string_literal: true

class Foo
  class Foo
  end
end

foo = Foo.new
#     ^ apply-rename: [A] newName: Bar
```

The `apply-rename` assertion here says "simulate a user starting a rename from
the position of this caret." You'll need to add an `.rbedited` file that reflects
what the result of the changes should look like. In this case, the file would look
like this:

```ruby
# -- test/testdata/lsp/refactor/mytest.A.rbedited --

# typed: true
# frozen_string_literal: true

class Bar
  class Foo
  end
end

foo = Bar.new
#     ^ apply-rename: [A] newName: Bar
```

You can test that invalid renames aren't applied by adding `invalid: true` to your
test, like so:
```ruby
# -- test/testdata/lsp/refactor/mytest.rb --

# typed: true
# frozen_string_literal: true

class Foo
  class Foo
  end
end

foo = Foo.new
#     ^ apply-rename: [A] newName: foo invalid:true
```

To test for a specific error message, add an `expectedErrorMessage` argument to the test:
```ruby
# typed: true
# frozen_string_literal: true

require_relative './constant__class_definition.rb'

sig { params(foo: Foo::Foo).returns(Foo::Foo) }
def foo(foo); end

class Baz
#     ^ apply-rename: [D] newName: Bar invalid: true expectedErrorMessage: Renaming constants defined in .rbi files is not supported; symbol Baz is defined at test/testdata/lsp/rename/constant__rbi_class_reference.rbi

end
```

You can add more files that reference the constant you're renaming, just make sure
to add a matching `.rbedited` file with the same version.

#### Testing incremental type checking

In LSP mode, Sorbet runs file updates on a *fast path* or a *slow path*. It checks the structure of the
file before and after the update to determine if the change is covered under the fast path. If it is,
it performs further processing to determine the set of files that need to be type checked.

LSP tests can define file updates in `<name>.<version>.rbupdate` files which contain the contents of `<name>.rb`
after the update occurs. For example, the file `foo.1.rbupdate` contains the updated contents of `foo.rb`.

If the test contains multiple files by using a `__` suffixed prefix, then all rbupdates with the same version will
be applied in the same update. For example, `foo__bar.1.rbupdate` and `foo__baz.1.rbupdate` will be applied
simultaneously to update `foo__bar.rb` and `foo__baz.rb`.

Inside `*.rbupdate` files, you can assert that the slow path ran by adding a line with `# assert-slow-path: true`.
You can assert that the fast path ran on `foo__bar.rb` and `foo__baz.rb` with
`#assert-fast-path: foo__bar.rb,foo__baz.rb`.

### LSP recorded tests

It is possible to record an LSP session and use it as a test. We are attempting to move away from this form of
testing, as these tests are hard to update and understand. If at all possible, try to add your test case as a
regular LSP test.

Any folder `<name>` that is added to `test/lsp/` will become a test.
This folder should contain a file named `<folderName>.rec` that contains a
recorded LSP session.

- Lines that start with "Read:" will be sent to sorbet as input.
- Lines that start with "Write:" will be expected from sorbet as output.

### Updating tests

Frequently when a test is failing, it's because something inconsequential
changed in the captured output, rather than there being a bug in your code.

To recapture the traces, you can run

```
tools/scripts/update_exp_files.sh
```

You will probably want to look through the changes and `git checkout` any files
with changes that you believe are actually bugs in your code and fix your code.

`update_exp_files.sh` updates every snapshot file kind known to Sorbet. This can
be slow, depending on what needs to be recompiled and updated. Some faster
commands:

```bash
# Only update the `*.exp` files in `test/testdata`
tools/scripts/update_testdata_exp.sh

# Only update the `*.exp` files in `test/testdata/cfg`
tools/scripts/update_testdata_exp.sh test/testdata/cfg

# Only update a single exp file's test:
tools/scripts/update_testdata_exp.sh test/testdata/cfg/next.rb

# Only update the `*.out` files in `test/cli`
bazel test //test/cli:update

# Update the `*.exp` files in `gems/sorbet/test/hidden-method-finder`
gems/sorbet/test/hidden-method-finder/update_hidden_methods_exp.sh
```


## Debugging

In general,

- to debug a normal build of sorbet?
  - `lldb bazel-bin/main/sorbet -- <args> ...`
  - (Consider using `--config=static-libs` for better debug symbols)
  - If you see weird Python errors on macOS, try `PATH=/usr/bin lldb`.
- to debug an existing Sorbet process (i.e., LSP)
  - launch Sorbet with the `--wait-for-dbg` flag
  - `lldb -p <pid>`
  - set breakpoints and then `continue`

Also, it’s good to get in the practice of fixing bugs by first adding an
`ENFORCE` (assertion) that would have caught the bug before actually fixing the
bug. It’s far easier to fix bugs when there’s a nice error message stating what
invariant you’ve violated. `ENFORCE`s are free in the release build.

## Writing docs

The sources for Sorbet's documentation website live in the
[`website/`](website/) folder. Specifically, the docs live in
[`website/docs/`](website/docs/), are all authored with Markdown, and are built
using [Docusaurus](https://docusaurus.io/).

[→ website/README.md](website/README.md)

^ See here for how to work with the documentation site locally.


## Editor and environment

### Bazel

Bazel supports having a persistent cache of previous build results so that
rebuilds for the same input files are fast. To enable this feature, run these
commands to create a `./.bazelrc.local` and cache folder:

```shell
# The .bazelrc.local will live in the sorbet repo so it doesn't interfere with
# other bazel-based repos you have.
echo "build  --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc.local
echo "test   --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc.local
mkdir -p "$HOME/.cache/sorbet/bazel-cache"
```

### Multiple git worktrees

Sometimes it can be nice to have [multiple working trees] in Git. This allows
you to have multiple active checkouts Sorbet, sharing the same `.git/` folder.
To set up a new worktree with Sorbet:

```shell
tools/scripts/make_worktree.sh <worktree_name>
```

### Shell

Many of the build commands are very long. You might consider shortening the
common ones with shell aliases of your choice:

```shell
# mnemonic: 's' for sorbet
alias sb="bazel build //main:sorbet --config=dbg"
alias st="bazel test //... --config=dbg --test_output=errors"
```

### Formatting files

We ensure that C++ files are formatted with `clang-format` and that Bazel BUILD
files are formatted with `buildifier`. To avoid inconsistencies between
different versions of these tools, we have scripts which download and run these
tools through `bazel`:

```
tools/scripts/format_cxx.sh
tools/scripts/format_build_files.sh
```

CI will fail if there are any unformatted files, so you might want to set up
your files to be formatted automatically with one of these options:

1.  Set up a pre-commit / pre-push hook which runs these scripts.
2.  Set up your editor to run these scripts. See below.


### Editor setup for C++

The `clang` suite of tools has a pretty great story around editor tooling: you
can build a `compile_commands.json` using Clang's [Compilation Database] format.

Many clang-based tools consume this file to provide language-aware features in,
for example, editor integrations.

To build a `compile_commands.json` file for Sorbet with bazel:

```
tools/scripts/build_compilation_db.sh
```

This builds a `./compile_commands.json` file (that is gitignored). This file
hard-codes some paths into the Bazel sandbox. These files can get stale,
especially when they're generated by Bazel `genrule`'s. In particular, the
`./compile_commands.json` references files in Bazel's `dbg` configuration (e.g.,
whatever was last built with `-c opt` / `--compilation_mode=opt`). If you're
seeing stale errors, consider running a command like `./bazel build
//main:sorbet -c opt`.

You are encouraged to play around with various clang-based tools which use the
`compile_commands.json` database. Some suggestions:

-   [rtags] -- Clang aware jump-to-definition / find references / etc.

    ```shell
    brew install rtags

    # Have the rtags daemon be automatically launched by macOS on demand
    brew services start rtags

    # cd into sorbet
    # ensure that ./compile_commands.json exists

    # Tell rtags to index sorbet using our compile_commands.json file
    rc -J .
    ```

    There are rtags editor plugins for most text editors.

-   [clangd] -- Clang-based language server implementation

    `clangd` supports more features than `rtags` (specifically, reporting
    Diagnostics), but can be somewhat slower at times because it does not
    pre-index all your code like rtags does.

    After successfully compiling Sorbet, point your editor to use the
    `clangd` executable located in
    `bazel-sorbet/external/llvm_toolchain_10_0_0/bin/clangd`.

-   [clang-format] -- Clang-based source code formatter

    We build `clang-format` in Bazel to ensure that everyone uses the same
    version. Here's how you can get `clang-format` out of Bazel to use it in
    your editor:

    ```shell
    # Build clang-format with bazel
    ./bazel build //tools:clang-format

    # Once bazel runs again, this symlink to clang-format will go away.
    # We need to copy it out of bazel so our editor can use it:
    mkdir -p "$HOME/bin"
    cp bazel-bin/tools/clang-format $HOME/bin

    # (Be sure that $HOME/bin is on your PATH, or use a path that is)
    ```

    With `clang-format` on your path, you should be able to find an editor
    plugin that uses it to format your code on save.

    Note: our format script passes some extra options to `clang-format`.
    Configure your editor to pass these options along to `clang-format`:

    ```shell
    -style=file -assume-filename=<CURRENT_FILE>
    ```

-   [CLion] -- JetBrains C/C++ IDE

    CLion can be made aware of the `compile_commands.json` database.
    Replaces your entire text editing workflow (full-fledged IDE).

-   [vscode-clangd] -- Clangd extension for VS Code

    This extension integrates clangd (see above) with VS Code. It will also
    run `clang-format` whenever you save. **Note: Microsoft's C/C++ extension
    does *not* work properly with Sorbet's `compile_commands.json`.**

    The settings for this repository automatically configure vscode-clangd to
    run the clangd executable in the `bazel-sorbet` directory. Note that you
    will need to compile Sorbet once before it will work.

    clangd operates on `compile_commands.json`, so make sure you run the
    `./tools/scripts/build_compilation_db.sh` script.

[Compilation Database]: https://clang.llvm.org/docs/JSONCompilationDatabase.html
[rtags]: https://github.com/Andersbakken/rtags
[clangd]: https://clang.llvm.org/extra/clangd.html
[clang-format]: https://clang.llvm.org/docs/ClangFormat.html
[CLion]: https://www.jetbrains.com/clion/
[vscode-clangd]: https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd

Here are some sample config setups:

- Vim
  - [rtags (vim-rtags)](https://github.com/jez/dotfiles/blob/dafe23c95fd908719bf477f189335bd1451bd8a7/vim/plug-settings.vim#L649-L676)
  - [clangd + clang-format (ALE)](https://github.com/jez/dotfiles/blob/dafe23c95fd908719bf477f189335bd1451bd8a7/vim/plug-settings.vim#L288-L303)
  - [clangd + clang-format (coc.nvim)](https://github.com/elliottt/vim-config/blob/35f328765528f6b322fb7d5a03fb3edd81067805/coc-settings.json#L3-L15)
