# gems/sorbet/

This folder contains the source for the `'sorbet'` gem. This gem serves two
purposes:

- It depends on `'sorbet-static'`, which includes the release executable of the
  Sorbet static type checker. Thus adding `'sorbet'` to a Gemfile fetches a
  pre-built version of Sorbet behind the scenes.

- It contains the `srb` executable. This is a wrapper script that we use for:

  - initializing a Sorbet project / generating RBI files
  - finding & running the Sorbet executable in `'sorbet-static'` to type check a
    project

This README is for contributors. Here are some docs for how to use this gem:

- <https://sorbet.org/docs/adopting>
- <https://sorbet.org/docs/rbi>


## Project structure

A quick overview of the project structure:

```
.
├── bin/
│   ├── srb              → The main CLI entry point (Bash)
│   └── srb-rbi          → The main Ruby entrypoint (called by `srb`)
├── lib/                 → Supporting files for `srb-rbi`
│   └── ···
├── test/
│   ├── ···
│   └── snapshot/        → Snapshot tests for `srb init`
│       ├── partial/
│       └── total/
├── Gemfile
└── sorbet.gemspec
```

- The main entry point is `bin/srb`. It's written in Bash in an attempt to avoid
  the performance cost of forking a Ruby process just to type check.

  (Forking a Ruby process = ~200ms; Sorbet can frequently type check an entire
  project in as much time.)

- When generating RBI files (i.e., not type checking a project), `bin/srb` calls
  into `bin/srb-rbi`, which as a number of subcommands. This is where all the
  actual RBI generation logic is.


## Environment variables

There are a number of environment variables that `srb` reads from to change it's
behavior:

- `SRB_SORBET_EXE`

  Overrides the `sorbet` executable used by all calls to `srb tc`.
  (The default is to find and use the version inside the `'sorbet-static'` gem.)

- `XDG_CACHE_HOME`

  The `srb` command keeps a cache of `sorbet-typed` on disk. Setting
  `XDG_CACHE_HOME` will change the folder where the `sorbet-typed` cache will
  be. (The default when this is unset is to use `$HOME/.cache/`.)

- `SRB_SORBET_TYPED_REVISION`

  Pin the sorbet-typed cache to a specific revision. (The default is to fetch
  and use the latest `master` commit.)

- `SRB_SKIP_GEM_RBIS`

  Disables the loading of
  [RBI files exported from gems](https://sorbet.org/docs/rbi#rbis-within-gems).
  This allows the LSP mode to work properly even when some gems in the Gemfile
  are exporting RBI files.

## Running locally

To run `srb` locally requires first being able to build the `sorbet` executable
itself. See the top-level README.md in this repo.

Once you've built `sorbet`, to run `srb`:

```
❯ SRB_SORBET_EXE=bazel-bin/main/sorbet bin/srb ...
```

To make this easier, you can either

- put `sorbet` on your `PATH` (see the top-level README.md for suggestions), or
- write a test, and use the test harness (see [Running tests](#running-tests)).


## Testing

Due to the changes announced in [this blog post], we no longer support `srb
rbi`. As such, we've deleted the `srb init` and `srb rbi` tests, as continuing
to maintain them proved to be burdensome.

[this blog post]: https://sorbet.org/blog/2022/07/27/srb-tapioca
