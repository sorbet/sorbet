---
id: adopting
title: Adopting Sorbet in an Existing Codebase
sidebar_label: Adopting Sorbet
---

> **Heads up**: This process still requires a fair bit of manual work, and is
> one of the main reasons why Sorbet is still in private beta.

Here well outline the high level steps to start adopting Sorbet. As you have
questions with this guide, please ask in the Sorbet slack, and feel free to
contribute improvements!


## Step 1: Install dependencies

<!-- TODO(jez) How to install sorbet from a package manager / gem -->

There are two components to Sorbet: the static and runtime systems. They are
currently installed separately:

1.  Download the `sorbet` binary.

    Since Sorbet is still pre-release, it is not in any package managers.\
    It can be downloaded from the [Releases] page.

    It's best to put this somewhere that's on your `$PATH`.

1.  Reference the `sorbet-runtime` gem in your `Gemfile`:

    ```ruby
    # -- Gemfile --
    # TODO(jez) Update these instructions after moving sorbet-runtime to monorepo
    gem 'sorbet-runtime'
    ```

1.  Install `sorbet-runtime` using bundler:

    ```shell
    ❯ bundle install
    ```

[Releases]: https://github.com/stripe/sorbet/releases

To test that everything is working at this point, you should be able to run
these two commands, and see output like this:

```plain
❯ sorbet -e 'puts "Hello"'
No errors! Great job.
```

```plain
❯ bundle exec ruby -e 'puts(require "sorbet-runtime")'
true
```


## Step 2: Prepare the file list

Sorbet makes no attempt to infer which files make up a given Ruby project. So
before we can run `sorbet` for the first time, we need to list all the files in
our project. One way to do this is using `find`:

```bash
❯ find . -name '*.rb' -o -name '*.rbi' > FILE_LIST
```

Some notes about what this command does:

- It lists all files under the current folder ending in `*.rb` or `*.rbi`
- (We'll cover what `*.rbi` files are in a moment)
- The files are stored in the `FILE_LIST` file (feel free to pick a different
  name).

Each project is different, so this `find` command might not work for everyone.
Feel free to tailor it to meet your project's needs.[^file-list]

[^file-list]: For context, the file list at Stripe ties into our mono-repo build
procedure. Our builds use code generation to create Ruby files in various
places. The way we build our file list reflects our build procedure.

To test that everything is working at this point, you should see a file called
`FILE_LIST` in the current directory, and it should have a bunch of lines, each
one with a path to a Ruby file in your project. For example:

```bash
❯ head FILE_LIST
lib/foo.rb
lib/bar.rb
test/runner.rb
...
```

> **Warning**: Every file in our project should be accounted for in the file
> list. Sorbet is not designed to work if files are withheld.


## Step 3: Run `sorbet` for the first time

Now we can run `sorbet` by putting an `@` sign[^at-sign] in front of the name of
our file list:

[^at-sign]: Prefixing the file list's name with an `@` tells Sorbet to read that
file and treat each line as a separate argument. We could have instead passed
filenames directly to `sorbet`, but there's a limit to how many files can be
passed at the command line.

```bash
❯ sorbet @FILE_LIST
```

At this point, it's likely that you'll see LOTS of error reported by `sorbet`.
Our next job is to start fixing these errors.


## Step 4: Fix constant resolution errors

Fixing the initial errors reported by Sorbet is the biggest hurdle to adoption.
Empirically, there are a handful of categories of errors people encounter at
this step:

1.  Parse errors

    Sorbet requires that all files parse as valid Ruby syntax.

1.  Dynamic `include`s

    Sorbet cannot statically analyze a codebase that dynamically `include`s
    code. For example, [code like this][rand-include] is impossible to
    statically analyze.

    Dynamic includes must be rewritten so that all `include`s are constant
    literals.

1.  Missing constants

    For Sorbet to be effective, it has to know about every class, module, and
    constant that's defined in a codebase. Constants are central to
    understanding the inheritance hierarchy and to validating type annotations.

[rand-include]: https://sorbet.run/#module%20A%3B%20end%0Amodule%20B%3B%20end%0A%20%20%0Adef%20x%0A%20%20rand.round%20%3D%3D%200%20%3F%20A%20%3A%20B%0Aend%0A%20%20%0Aclass%20Main%0A%20%20include%20x%0Aend

To solve points (2) and (3), let's introduce [`*.rbi` files](rbi.md). We saw
`*.rbi` files mentioned before when we discussed building the `FILE_LIST`. RBI
stands for "Ruby Interface." `*.rbi` files are purely type annotations, separate
from runnable Ruby code.

There is currently no official way to generate `*.rbi` files to ease the
adoption of Sorbet. For some tips, see [RBI Files](rbi.md).

> **Note**: This procedure is likely to change greatly while Sorbet is in
> private beta.

<!-- TODO(jez) Replace these instructions when we have a concrete way to generate rbi's -->


## Step 5: Enable type checking

At this step, running `sorbet @FILE_LIST` should show zero errors.

Congrats! Step 4 was the biggest hurdle to adopting Sorbet. To recap, Sorbet now
enforces that...

- all Ruby files parse.

- we know every class, module, and constant in a codebase. These can be 100%
  accurately shown in auto-complete suggestions and used in type annotations.

- all gems have explicit interfaces. In addition to yard documentation, `*.rbi`
  files are machine-checked documentation about the libraries we're using.

But importantly, Sorbet does **not** yet report type errors. The final step is
to start enabling more checks in our code by turning on type checking. The rest
of this site is dedicated to learning about the extra checks you can enable. The
tl;dr is that type checking happens when we add [`# typed: true`](static.md) to
files and write [method signatures](sigs.md) in those files.

## What next?

- [Enable Static Checks](static.md)

  At this point, Sorbet is currently silencing all type-related errors, and only
  knows about the types coming from the standard library. Adding type
  annotations lets Sorbet find more bugs in our code and provide better
  completion results.

- [Enable Runtime Checks](runtime.md)

  Sorbet relies heavily on runtime type checks to back up the predictions made
  statically about a codebase. Learn why these checks are necessary, and how to
  enable them.

- [Signatures](sigs.md)

  Sorbet requires some type annotations in order to work. The most common kind
  of annotations are method signatures, or `sig`s.
