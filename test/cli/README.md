# Driver tests

This directory contains tests for the `ruby-typer` driver. Tests of
the actual typechecking passes should prefer the `test/testdata/`
machinery, but this directory allows for testing the driver binary
end-to-end and command-line flags.

# Test organization

A CLI test consists, at minimum, of a subdirectory in this directory,
and a `.sh` and `.out`, organized like so:

- `cli/$NAME/$NAME.sh`
- `cli/$NAME/$NAME.out`

It may optionally contain any number of `.rb` or `.yaml` files in the
`cli/$NAME/` directory.

The Bazel rules in this directory will produce two targets:
`run_$NAME` and `test_$NAME`:

- `bazel run run_$NAME` will execute the `.sh`
- `bazel test test_$NAME` will execute the `.sh` and compare the
  output to the `.out` file, succeeding if identical, and printing a
  unified diff otherwise.

Scripts are all run inside Bazel, and so will be executed from the top
of the workspace, and can access both source files and built targets
using their path from the root. In particular, the compiled
`ruby-typer` binary is available under `main/ruby-typer`.

`tools/scripts/update_exp_files.sh` will use `bazel query` to discover
all `run_*` targets, and then execute them and update the `.out`
files.
