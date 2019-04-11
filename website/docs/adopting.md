---
id: adopting
title: Adopting Sorbet in an Existing Codebase
sidebar_label: Adopting Sorbet
---

> **Heads up**: Sorbet is still in a private beta while we iron out the
> onboarding experience. You'll only be able to follow these instructions if
> you've been given access to the private beta.
>
> Interested in signing up? Email us at <sorbet@stripe.com>.


## Step 1: Install dependencies

There are two components to Sorbet: the command line interface and runtime
systems. We'll declare them in our Gemfile and install them with Bundler:

```ruby
# -- Gemfile --

source 'https://stripe.dev/sorbet-repo/super-secret-private-beta/' do
  gem 'sorbet', :group => :development
  gem 'sorbet-runtime'
end
```

```plaintext
❯ bundle install
```

### Verify installation

To test that everything is working so far, we can run these commands:

```plaintext
❯ srb
[help output]

❯ srb typecheck -e 'puts "Hello, world!"'
No errors! Great job.

❯ bundle exec ruby -e 'puts(require "sorbet-runtime")'
true
```


## Step 2: Initialize Sorbet in our project

For small projects, Sorbet can run on a single file with no additional
information. But for projects that have multiple files and depend on other gems,
Sorbet needs to know more information to work.

To initialize Sorbet in an existing project, run:

```plaintext
❯ srb init
```

It's normal for this command to spew a bunch of output to the terminal. When
it's done, there should be a `sorbet/` folder in the current directory. Be sure
to check the entire folder into version control.


### Verify initialization

The contents of the `sorbet/` folder should now look like this:

```plaintext
sorbet/
├── config
└── rbi/
    └── ···
```

- `sorbet/config` is the config file Sorbet will read (see [Command Line
  Reference](cli.md))

- `sorbet/rbi/` is a folder containing [RBI files](rbi.md). RBI files (or "Ruby
  Interface" files) declare classes, modules, constants, and methods to Sorbet
  that it can't see on its own.

  `srb init` creates many kinds of RBI files. For more information, see [RBI
  files](rbi.md).

If these two items exist, we're all set to typecheck our project for the first
time.


## Step 3: Run `srb tc` for the first time

Now that we've initialized Sorbet, type checking Sorbet should be as simple as:

```plaintext
❯ srb tc
```

<!-- TODO(jez) It's hard to describe succintly which files will be checked if we
     suggest-typed by default and ignore files -->

By default, this will type check every Ruby file in the current folder. To
configure this, see [Command Line Reference](cli.md).


## Step 4: Fix constant resolution errors

<!-- TODO(jez) How to unsilence the errors in ignored files. -->

At this point, it's likely that there are lots of errors in our project, but
Sorbet **silences** them by default. Our next job is to unsilence them and then
fix the root causes. Empirically, there are a handful of categories of errors
people encounter at this step:

1.  Parse errors

    Sorbet requires that all files parse as valid Ruby syntax.

2.  Dynamic constant references

    Sorbet does not support resolving constants through expressions. For
    example, `foo.bar::A` is not supported---all constants must be resolvable
    without knowing what type an expression has. In most cases, it's possible to
    rewrite this code to use method calls (like, `foo.bar.get_A`).

3.  Dynamic `include`s

    Sorbet cannot statically analyze a codebase that dynamically `include`s
    code. For example, [code like this][rand-include] is impossible to
    statically analyze.

    Dynamic includes must be rewritten so that all `include`s are constant
    literals.

4.  Missing constants

    For Sorbet to be effective, it has to know about every class, module, and
    constant that's defined in a codebase, including all gems. Constants are
    central to understanding the inheritance hierarchy and thus knowing which
    types can be used in place of which other types.

[rand-include]: https://sorbet.run/#module%20A%3B%20end%0Amodule%20B%3B%20end%0A%20%20%0Adef%20x%0A%20%20rand.round%20%3D%3D%200%20%3F%20A%20%3A%20B%0Aend%0A%20%20%0Aclass%20Main%0A%20%20include%20x%0Aend

To solve points (3) and (4), Sorbet uses [RBI files](rbi.md). We mentioned RBI
files before when we introduced `srb init`. RBI files are purely annotations
files, separate from Ruby source code. While `srb init` can automatically create
these files, it's not a perfect process. To eliminate constant errors, sometimes
Sorbet requires hand-written RBI files.

To learn more about RBI files and how they can help with adopting Sorbet, see
[the docs here](rbi.md).


## Step 5: Enable type checking

At this step, running `srb tc` should show zero errors.

Congrats! Step 4 was the biggest hurdle to adopting Sorbet. To recap, Sorbet now
enforces that...

- all Ruby files parse.

- every class, module, and constant in a codebase is known. These can be 100%
  accurately shown in auto-complete suggestions and used in type annotations.

- all gems have explicit interfaces. More than YARD documentation, RBI files
  are machine-checked documentation about the libraries we're using.

Importantly, Sorbet does **not** yet report **type errors** (the errors we've
seen so far have been syntax errors and constant resolution errors). The final
step is to start enabling more type checks in our code.

The rest of this site is dedicated to learning about the extra checks we can
enable. The tl;dr is that type checking happens when we add [`# typed:
true`](static.md) to files and write [method signatures](sigs.md) in those
files.

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
