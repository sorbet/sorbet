# Sorbet

This repository contains Sorbet, a static typechecker for a subset of Ruby. It
is still in early stages, but is mature enough to run on the majority of Ruby
code at Stripe. You are welcome to try it, though, but your experience might
still be rough.

This README contains documentation specific to developing in Sorbet. You might
also want to see:

- The Sorbet [design doc](https://hackpad.corp.stripe.com/Design-Doc-sorbet-zd1LGHPfpvW)
- The Sorbet [user guide](http://go/types)

## Quickstart

1.  Install the dependencies

    - macOS: `brew install bazel ragel bison autoconf coreutils parallel`
    - ubuntu: `sudo apt install ragel bison libncurses5-dev autoconf`
      - then: `cat ./bazelrc-ubuntu >> ./.bazelrc`

2. Build Sorbet

    - `bazel build //main:sorbet --config=dbg`

3. Run Sorbet!

    - `bazel-bin/main/sorbet -e "42 + 'hello'"`


### Troubleshooting

If you see a build error that looks like

```
Executing genrule @parser//:bison_parser failed
```

then you might need to run

```
brew link bison --force
```

to symlink the correct version of bison into `/usr/local/bin` so it appears on
your PATH.

## Building Sorbet

There are multiple ways to build `sorbet`. This one is the most common:

```
bazel build //main:sorbet --config=dbg
```

This will build an executable in `bazel-bin/main/sorbet` (see "Running Sorbet"
below). There are many options you can pass when building `sorbet`:

- `--config=dbg`
  - Most common build config for development.
  - Fast, good stack traces, and debugger symbols.
- `--config=sanitize`
  - Link in extra sanitizers, in particular: UBSan and ASan.
  - Catches most memory and undefined-behavior errors.
  - Substantially larger and slower binary.
- `--config=debugsymbols`
  - (Included by `--config=dbg`) debugging symbols, and nothing else.
- `--config=forcedebug`
  - Use more memory, but report even more sanity checks.
- `--config=release`
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

There are three ways to run your changes to `sorbet` over pay-server. Two of
them leverage CI, and one of them runs on your laptop.

1.  Bump the SHA in [ci/stripe-internal-sorbet-pay-server-sha]

    - CI jobs already run Sorbet against pay-server every push.
    - Bumping the SHA in this file asks sorbet to run against a newer
      pay-server.

2.  Bump the SHA in [sorbet/sorbet.sha] (in pay-server)

    - This is the version of sorbet that runs over every pay-server CI job.
    - Updating this file is essentially "releasing" a new version of sorbet to
      Stripe.

3. Use the `sorbet/scripts/typecheck_devel` script in pay-server.

    - This requires a bit more setup. See the section below on "Local
      development"

[ci/stripe-internal-sorbet-pay-server-sha]: https://git.corp.stripe.com/stripe-internal/sorbet/blob/master/ci/stripe-internal-sorbet-pay-server-sha
[sorbet/sorbet.sha]: https://git.corp.stripe.com/stripe-internal/pay-server/blob/master/sorbet/sorbet.sha


## Writing tests

We write tests by adding files to subfolders of the `test/` directory.
Individual subfolders are "magic"; each contains specific types of tests.

### Expectation tests

Any file `<name>.rb` that is added to `test/testdata` becomes a test. The file
must either:

- typecheck entirely, or
- throw errors **only** on lines marked lines.

Mark that a line should have errors with `# error: <message>` (the `<message>`
must match the raised error message). In case there are multiple errors on this
line, use `MULTI` for your `<message>`.

You can optionally create `<name>.rb.<phase>.exp` files that contain pretty
printed internal state after phase `<phase>`. The tests will ensure that the
internal state exactly matches this snapshot.

### CLI tests

Any folder `<name>` that is added to `test/cli/` becomes a test.
This folder should have a file `<name>.sh` that is executable.
When run, its output will be compared against `<name>.out` in that folder.

Our bazel setup will produce two targets:

- `bazel run run_<name>` will execute the `.sh` file
- `bazel test test_<name>` will execute the `.sh` and check it against what's in
  the `.out` file.

The scripts are run insize Bazel, so they will be executed from the top of the
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

## Local development

There are a couple hoops to jump through for local development, but it's worth
it because at the end you'll be able to run `sorbet` under `lldb` running over
`pay-server`.

### Build `sorbet` with optimizations

To run on pay-server, you must be using an optimized build (any build with `-c
opt`). For example:

```
bazel build //main:sorbet -c opt
```

Alternatively, if you want to exactly reproduce what we ship to our users:

```
bazel build //main:sorbet --config=release
```

(but note that `--config=release` has poor symbols because `-O2` is very
aggressive, and no sanitization).

Also note that `--config=release` on Linux makes a fully static binary. This
means that this binary can potentially run on any Linux distribution, but it has
some quirks due to glibc having long-known bugs when linked statically (e.g. it
can't resolve DNS names).

### Set up "autogen" locally

Officially, local autogen is not supported. Welcome to Developer Productivity:
local "autogen" is supported for you, by you. Sorbet depends on having a list of
all Ruby files to run over, which we can get by running one of the intermediate
steps of autogen locally:

```
bundle exec rake build:FileListStep
```

This might fail for any number of reasons, but the error messages are usually
decent if you need to troubleshoot.

The generated file list will frequently get out of date, so you'll want to
re-run it every time you have to run sorbet locally over pay-server.

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
`sorbet` locally over pay-server:

```
sorbet/scripts/typecheck_devel
```

If it crashes with a message about a "bus error," probably you forgot to build
with optimizations. See step 1.

If you're debugging a crash, you might also want to set `SORBET_RUN_UNDER`:

```
SORBET_RUN_UNDER="lldb --" sorbet/scripts/typecheck_devel
```

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
  - `lldb -p <pid>`

Also, it’s good to get in the practice of fixing bugs by first adding an
`ENFORCE` (assertion) that would have caught the bug before actually fixing the
bug. It’s far easier to fix bugs when there’s a nice error message stating what
invariant you’ve violated. `ENFORCE`s are free in the release build.

### Profiling

- [ ] TODO(jez) Write about how to profile Sorbet


## Editor and environment

### Bazel

Bazel supports having a persistent cache of previous build results so that
rebuilds for the same input files are fast. To enable this feature, run these
commands to create a `~/.bazelrc` and cache folder:

```
echo "build  --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc
echo "test   --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ./.bazelrc
mkdir -p "$HOME/.cache/sorbet/bazel-cache"
```

### Shell

Many of the build commands are very long. You might consider shortening the
common ones with shell aliases of your choice:

```
# mnemonic: 's' for sorbet
alias sb="bazel build //main-sorbet --config=dbg"
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
your files to be formatted automatically. There are two options:


1.  Set up a pre-commit / pre-push hook which runs these scripts and commits any
    changes.
2.  Run the script once locally, then copy `clang-format` onto your PATH and
    point your editor at it.

    - The `format_cxx.sh` script builds `bazel-bin/tools/clang-format`.
    - All bazel build targets share `bazel-bin`, so building anything else
      invalidates the `clang-format` symlink.
    - Copying `clang-format` onto your PATH works around this detail, so that
      your editor can always run it.


## Updating sorbet.run

We have an online REPL for Sorbet: <https://sorbet.run>. It's deployed as a
GitHub pages site, and so the sources lives on github.com. Make sure you've
cloned it locally:

```
git clone https://github.com/stripe/sorbet.run ~/stripe/sorbet.run
```

To build the WASM and asm.js assets, follow these steps (macOS-specific):

1.  `brew install emscripten cmake`

    We're not using `cmake` directly--it's used transitively by the `emcc`
    command.

1.  Take note of the caveats in `brew info emscripten`.

    In particular, after your first build (successful or unsuccessful), you will
    have to edit some variables `~/.emscripten` to point at the proper paths
    according to Homebrew.

1.  `emscripten/build.sh`

    Once this command finishes and everything is built, it will print a manual
    step with where to copy the files into the sorbet.run repo.

Note that the emscripten build is not run in CI, so it frequently becomes broken
with time. If you get stuck building it, please ask on Slack!

