<p align="center">
  <img alt="Sorbet logo" width="200" src="docs/logo/sorbet-logo-purple-sparkles.svg">
</p>

# Sorbet

This repository contains Sorbet, a static typechecker for a subset of Ruby. It
is still in early stages, but is mature enough to run on the majority of Ruby
code at Stripe. You are welcome to try it, though, but your experience might
still be rough.

This README contains documentation specific to developing in Sorbet. You might
also want to see:

- The Sorbet [design doc](https://hackpad.corp.stripe.com/Design-Doc-sorbet-zd1LGHPfpvW)
- The Sorbet [user guide](http://go/types)
- The [Sorbet internals](https://stripe.exceedlms.com/student/activity/372979) talk
- The [Gradual Typing of Ruby at Scale](https://www.youtube.com/watch?v=uFFJyp8vXQI) talk

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Table of Contents

- [Sorbet user-facing design principles](#sorbet-user-facing-design-principles)
- [Quickstart](#quickstart)
- [Building Sorbet](#building-sorbet)
  - [Common Compilation Errors](#common-compilation-errors)
- [Running Sorbet](#running-sorbet)
- [Running the tests](#running-the-tests)
- [Testing Sorbet against pay-server](#testing-sorbet-against-pay-server)
- [Writing tests](#writing-tests)
  - [Expectation tests](#expectation-tests)
  - [CLI tests](#cli-tests)
  - [LSP tests](#lsp-tests)
  - [Updating tests](#updating-tests)
- [Running over pay-server locally](#running-over-pay-server-locally)
  - [Build `sorbet`](#build-sorbet)
  - [Set up "autogen" locally](#set-up-autogen-locally)
  - [Make sure `sorbet` is on your PATH](#make-sure-sorbet-is-on-your-path)
  - [Run `sorbet/scripts/typecheck_devel`](#run-sorbetscriptstypecheck_devel)
- [C++ conventions](#c-conventions)
- [Debugging and profiling](#debugging-and-profiling)
  - [Debugging](#debugging)
  - [Profiling](#profiling)
- [Writing docs](#writing-docs)
- [Updating sorbet.run](#updating-sorbetrun)
- [Editor and environment](#editor-and-environment)
  - [Bazel](#bazel)
  - [Shell](#shell)
  - [Formatting files](#formatting-files)
  - [Editor setup for C++](#editor-setup-for-c)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Sorbet user-facing design principles

Early in our project we've defiend some guidelines for how working with sorbet should feel like.

1. **Explicit**

    We're willing to write annotations, and in fact see them as
    beneficial; They make code more readable and predictable. We're here
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
    scales better, and -- most importantly -- is easiest for users to
    learn+understand.

4. **Compatible with Ruby**

    In particular, we don't want new syntax. Existing Ruby syntax means
    we can leverage most of our existing tooling (editors, etc). Also,
    the whole point here is to improve an existing Ruby codebase, so we
    should be able to adopt it incrementally.

5. **Scales**

    On all axes: in speed, team size, codebase size and time (not
    postponing hard decisions). We already work in large Ruby codebases, and it will
    only get larger.

6. **Can be adopted gradually**

    In order to make adoption possible at scale, we cannot require all
    the teams to adopt it at once, thus we need to support teams adopting it
    at different pace.

## Quickstart

1.  Install the dependencies

    - `brew install bazel autoconf coreutils parallel`

2. Build Sorbet

    - `bazel build //main:sorbet --config=dbg`

3. Run Sorbet!

    - `bazel-bin/main/sorbet -e "42 + 'hello'"`


## Building Sorbet

There are multiple ways to build `sorbet`. This one is the most common:

```
bazel build //main:sorbet --config=dbg
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
- `--config=release-mac` and `--config=release-linux`
  - Exact set of options that we ship to our users.

Independently of providing or omitting any of the above flags, you can turn on
optimizations for any build:

- `-c opt`
  - Enables `clang` optimizations (i.e., `-O2`)
  - Required to run over pay-server without stack overflowing.

These args are not mutually exclusive. For example, a common pairing when
debugging is

```
--config=dbg --config=sanitize
```

In tools/bazel.rc you can find out what all these options (and others) mean.

### Common Compilation Errors

**(Mac) `Xcode version must be specified to use an Apple CROSSTOOL`**

This error typically occurs after an XCode upgrade.

Developer tools must be installed, the XCode license must be accepted, and
your active XCode command line tools directory must point to an installed
version of XCode.

The following commands should do the trick:

```shell
# Install command line tools
xcode-select --install
# Ensure that the system finds command line tools in an active XCode directory
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
# Accept the XCode license.
sudo xcodebuild -license
# Clear bazel's cache, which may contain files generated from a previous
# version of XCode command line tools.
bazel clean --expunge
```

**(Mac) `fatal error: 'stdio.h' file not found`** (or some other system header)

This error can happen on Macs when the `/usr/include` folder is missing. The
solution is to install macOS headers via the following package:

```shell
open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg
```

## Running Sorbet

Run sorbet on an expression:

```
bazel-bin/main/sorbet -e "1 + false"
```

Run sorbet on a file:

```
bazel-bin/main/sorbet foo.rb
```

Running `bazel-bin/main/sorbet` `--``help` will show lots of options. These are
the common ones:


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
  - Note that this will be really slow on pay-server!
- `--wait-for-dbg`
  - Will freeze Sorbet on startup and wait for a debugger to attach
  - This is useful when you don't have control over launching the process (LSP)


## Running the tests

To run all the tests:

```
bazel test //... --config=dbg
```

(The `//...` literally means "all targets".)

To run a subset of the test that we normally run in development run:

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

There are two ways to run your changes to `sorbet` over pay-server:

1.  Push your Sorbet changes to a branch, and then make a PR to pay-server to
    bump the SHA in [sorbet/sorbet.sha].

    pay-server fetches this SHA, builds it on Stripe infrastructure, and then
    type checks pay-server with it.

2.  Use the `sorbet/scripts/typecheck_devel` script in pay-server to run a
    version of Sorbet built on your laptop.

    This requires a bit more setup. See the section below on [Running over
    pay-server locally](#running-over-pay-server-locally).

[ci/stripe-internal-sorbet-pay-server-sha]: https://git.corp.stripe.com/stripe-internal/sorbet/blob/master/ci/stripe-internal-sorbet-pay-server-sha
[sorbet/sorbet.sha]: https://git.corp.stripe.com/stripe-internal/pay-server/blob/master/sorbet/sorbet.sha


## Writing tests

We write tests by adding files to subfolders of the `test/` directory.
Individual subfolders are "magic"; each contains specific types of tests.
We aspire to have our tests be fully reproducible.
Note that in C++
> Hash functions are only required to produce the same result for the same input within a single execution of a program.

Thus we expect all user-visible outputs to be explicitly sorted using a
reliable key.

### test_corpus tests

These tests are either called [test_corpus] tests or [testdata] tests based on
the name of the test harness and folder where the tests are located,
respectively.


[test_corpus]: test/test_corpus.cc
[testdata]: test/testdata/

To create a test_corpus test, add any file `<name>.rb` to `test/testdata`, in
any folder depth. The file must either:

- typecheck entirely, or
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
representations of internal data structures, accoring to what would be printed
by `-p <phase>`. The snapshot must exactly match the output generated by running
`sorbet -p <phase> <name>.rb` for the test to pass.

You can run this test with:

```
bazel test //test:test_PosTests/testdata/path/to/<name>
```

### CLI tests

Any folder `<name>` that is added to `test/cli/` becomes a test.
This folder should have a file `<name>.sh` that is executable.
When run, its output will be compared against `<name>.out` in that folder.

Our bazel setup will produce two targets:

- `bazel run //test/cli:test_<name>` will execute the `.sh` file
- `bazel test //test/cli:test_<name>` will execute the `.sh` and check it against what's in
  the `.out` file.

The scripts are run inside Bazel, so they will be executed from the top of the
workspace and have access to sources files and built targets using their path
from the root. In particular, the compiled sorbet binary is available under
`main/sorbet`.

### LSP tests

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


## Running over pay-server locally

There are a couple hoops to jump through for local development, but it's worth
it because at the end you'll be able to run `sorbet` under `lldb` running over
`pay-server`.

### Build `sorbet`

There is a lot of code in pay-server, so it's best to build Sorbet with
optimizations before running over pay-server:

```
bazel build //main:sorbet -c opt --config=debugsymbols
```

Or, if you want to try running sorbet under LLDB:

```
bazel build //main:sorbet --config=dbg
```

Careful! This will build a version of Sorbet without optimizations and with
all ENFORCE checks enabled, so it will take longer to run. You might want to see
if you can reproduce and minimize the issue with an optimized build first, and
then use `--config=dbg` + LLDB on a minimized example.

Alternatively, if you want to exactly reproduce what we ship to our users:

```
bazel build //main:sorbet --config=release-mac
bazel build //main:sorbet --config=release-linux
```

(but note that `--config=release*` has poor symbols because `-O2` is very
aggressive, and no sanitization).

Also note that `--config=release-linux` on Linux makes a fully static binary.
This means that this binary can potentially run on any Linux distribution, but
it has some quirks due to glibc having long-known bugs when linked statically
(e.g. it can't resolve DNS names).

### Set up autogen locally

Officially, local autogen is not supported. Welcome to Developer Productivity:
local autogen is supported for you, by you. Sorbet depends on having a list of
all Ruby files to run over, which we can get by running autogen:

```
bundle install
bundle exec scripts/bin/autogen
```

This might fail for any number of reasons, but the error messages are usually
decent if you need to troubleshoot.

The generated files changes frequently. A good rule of thumb is: any time you
pull pay-server, you'll need to re-run autogen.

### Make sure `sorbet` is on your PATH

`sorbet` must be on your PATH for the next step. There are plenty of ways to do
this; here are two:

1.  Make a `~/bin` folder, and add it to your `PATH`.
    Then make a symlink to sorbet:

    ```
    cd ~/bin && ln -s ~/stripe/sorbet/bazel-bin/main/sorbet .
    ```

2.  Add sorbet's `bazel-bin/main` folder directly to your `PATH`.

### Run `sorbet/scripts/typecheck_devel`

Once you have an optimized build and the file list generated, you can run
`sorbet` locally using the `typecheck_devel` script in pay-server:

```
sorbet/scripts/typecheck_devel
```

If it crashes with a message about a "bus error," probably you forgot to build
with optimizations. See step 1.

If you're debugging a crash, you might also want to set `SORBET_RUN_UNDER`:

```
SORBET_RUN_UNDER="lldb --" sorbet/scripts/typecheck_devel
```

This will launch Sorbet under LLDB mode.

## C++ conventions

- [ ] TODO(jez) Write and link to "Notes on C++ Development"

- Use smart pointers for storage, references for arguments.
- No C-style allocators; use vectors instead.


## Debugging and profiling

### Debugging

In general,

- to debug a normal build of sorbet?
  - `lldb bazel-bin/main/sorbet` `--` `<args> ...`
- to debug something in pay-server?
  - See the section on “Local development” using `typecheck_devel`
- to debug an existing Sorbet process (i.e., LSP)
  - launch Sorbet with the `--wait-for-dbg` flag
  - `lldb -p <pid>`
  - set breakpoints and then `continue`

Also, it’s good to get in the practice of fixing bugs by first adding an
`ENFORCE` (assertion) that would have caught the bug before actually fixing the
bug. It’s far easier to fix bugs when there’s a nice error message stating what
invariant you’ve violated. `ENFORCE`s are free in the release build.

### Profiling

- [ ] TODO(jez) Write about how to profile Sorbet


## Writing docs

The sources for Sorbet's documentation website live in the
[`website/`](website/) folder. Specifically, the docs live in
[`website/docs/`](website/docs/), are all authored with Markdown, and are built
using [Docusaurus](https://docusaurus.io/).

[→ website/README.md](website/README.md)

^ See here for how to work with the documentation site locally.


## Updating sorbet.run

We have an online REPL for Sorbet: <https://sorbet.run>. It's deployed as a
GitHub pages site, and so the sources lives on github.com. Make sure you've
cloned it locally:

```
git clone https://github.com/stripe/sorbet.run ~/stripe/sorbet.run
```

Run `./tools/scripts/update-sorbet.run.sh` and follow the steps suggested by that script.


## Editor and environment

### Bazel

Bazel supports having a persistent cache of previous build results so that
rebuilds for the same input files are fast. To enable this feature, run these
commands to create a `./.bazelrc` and cache folder:

```shell
# The .bazelrc will live in the sorbet repo so it doesn't interfere with other
# bazel-based repos you have.
echo "build  --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc
echo "test   --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc
mkdir -p "$HOME/.cache/sorbet/bazel-cache"
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

    ```shell
    brew install llvm@8

    # => /usr/local/opt/llvm/bin/clangd
    # You might need to put this on your PATH to tell your editor about it.
    ```

    `clangd` supports more features than `rtags` (specifically, reporting
    Diagnostics), but can be somewhat slower at times because it does not
    pre-index all your code like rtags does.

-   [clang-format] -- Clang-based source code formatter

    We build `clang-format` in Bazel to ensure that everyone uses the same
    version. Here's how you can get `clang-format` out of Bazel to use it in
    your editor:

    ```shell
    # Build clang-format with bazel
    bazel build //tools:clang-format

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

    clangd will need to be on your path, or you will need to change the
    "clangd.path" setting.

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
