---
id: cli
title: Command Line Reference
---

Sorbet has a wealth of command line options; we'll only be covering a subset of
the available options here. For the full set of available options, check the
help at the command line.

In particular, this doc specifically covers the command line interface to the
`srb tc` subcommand. For information about `srb rbi`, see [RBI files](rbi.md).

## Config file

The first time `srb init` runs, it creates a config file that `srb tc` will
read, located at `sorbet/config`. The config file is actually just a
newline-separated list of arguments to pass to `srb tc`, the same as if they'd
been passed at the command line.

Every line in this file acts as if it's prepended to the argument list of `srb
tc`. So arguments in the config file are always passed first (if it exists), and
then arguments provided on the command line.

For a full description of the config file format, see the output of `srb tc
--help`.

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
`srb tc` will type check every file in the current directory[^gems]. Note:
Sorbet only checks files that end in `*.rb` or `*.rbi`. To check other files,
they must be explicitly named on the command line (or in the config file), or be
given an appropriate file extension.

[^gems]: This is incidentally why Sorbet does not type check gems: the paths to
gems' source directories are never given to Sorbet.

Sometimes, including an entire folder includes too many files. We can refine the
list of files Sorbet reads with the `--ignore` flag. The syntax is:

```plaintext
--ignore <pattern>
```

This will ignore input files that contain the given pattern in their paths
(relative to the input path passed to Sorbet). Patterns beginning with / match
against the prefix of these relative paths; others are substring matchs.
Matches must be against whole component parts, so `foo` matches `/foo/bar.rb`
and `/bar/foo/baz.rb` but not `/foo.rb` or `/foo2/bar.rb`.

## Overriding strictness levels

By default, Sorbet reads each file to determine it's [strictness
level](static.md#file-level-granularity-strictness-levels), defaulting to
`# typed: false` if none is provided.

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
- foo.rb
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
foo.rb:3: `Symbol(:"symbol")` doesn't match `String` for argument `arg0`
...
```
