---
id: faq
title: Frequently Asked Questions
sidebar_label: FAQ
---

## Why does Sorbet think this is `nil`? I just checked that it's not!

Sorbet implements a [flow-sensitive](flow-sensitive.md) type system, but there
are some [limitations](flow-sensitive.md#limitations-of-flow-sensitivity). In
particular, Sorbet does not assume a method called twice returns the same thing
each time!

See
[Limitations of flow-sensitivity](flow-sensitive.md#limitations-of-flow-sensitivity)
for a fully working example.

## It looks like Sorbet's types for the stdlib are wrong.

Sorbet uses [RBI files](rbi.md) to annotate the types for the Ruby standard
library. Every RBI file for the Ruby standard library is maintained by hand.
This means they're able to have fine grained types, but it also means that
sometimes they're incomplete or inaccurate.

The Sorbet team is usually too busy to respond to requests to fix individual
bugs in these type annotations for the Ruby standard library. That means there
are two options:

1.  Submit a pull request to fix the type annotations yourself.

    Every RBI file for the Ruby standard library lives [here][rbi-folder] in the
    Sorbet repo. Find which RBI file you need to edit, and submit a pull request
    on GitHub with the changes.

    This is the preferred option, because then every Sorbet user will benefit.

2.  Use an [escape hatch](troubleshooting.md#escape-hatches) to opt out of
    static type checks.

    Use this option if you can't afford to wait for Sorbet to be fixed and
    published. (Sorbet publishes new versions to RubyGems nightly).

If you're having problems making a change to Sorbet, we're happy to help on
Slack! See the [Community](/community) page for an invite link.

[rbi-folder]: https://github.com/sorbet/sorbet/tree/master/rbi

## What's the difference between `T.let`, `T.cast`, and `T.unsafe`?

[→ Type Assertions](type-assertions.md)

## What's the type signature for a method with no return?

```ruby
sig {void}
```

[→ Method Signatures](sigs.md)

## What's the signature for a method that returns multiple values?

Ruby's `return x, y` syntax is just shorthand for `return [x, y]`.

For methods using this multiple return shorthand, the return type should be
either:

- an [Array](stdlib-generics.md)

  ```ruby
  sig {return(T::Array[Integer])}
  def example
    return 1, 2
  end
  ```

- a [Tuple](tuples.md) (experimental)

  ```ruby
  sig {return([Integer, String])}
  def example
    return 1, ''
  end
  ```

[→ Method Signatures](sigs.md)

## How should I add types to methods defined with `attr_reader`?

Sorbet has special knowledge for `attr_reader`, `attr_writer`, and
`attr_accessor`. To add types to methods defined with any of these helpers, put
a [method signature](sigs.md) above the declaration, just like any other method:

```ruby
# typed: true
class A
  extend T::Sig

  sig {returns(Integer)}
  attr_reader :reader

  sig {params(writer: Integer).returns(Integer)}
  attr_writer :writer

  # For attr_accessor, write the sig for the reader portion.
  # (Sorbet will use that to write the sig for the writer portion.)
  sig {returns(Integer)}
  attr_accessor :accessor

  sig {void}
  def initialize
    @reader = T.let(0, Integer)
    @writer = T.let(0, Integer)
    @accessor = T.let(0, Integer)
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_reader%20%3Areader%0A%0A%20%20sig%20%7Bparams(writer%3A%20Integer).returns(Integer)%7D%0A%20%20attr_writer%20%3Awriter%0A%0A%20%20%23%20For%20attr_accessor%2C%20write%20the%20sig%20for%20the%20reader%20portion.%0A%20%20%23%20(Sorbet%20will%20use%20that%20to%20write%20the%20sig%20for%20the%20writer%20portion.)%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_accessor%20%3Aaccessor%0Aend">→
View on sorbet.run</a>

> Because Sorbet knows what `attr_*` methods define what instance variables, in
> `typed: strict` there are some gotchas that apply (which are the same for
> [all other uses](type-annotations.md) of instance variables): in order to use
> an instance variable, it must be initialized in the constructor, or be marked
> [nilable](nilable-types.md) (`T.nilable(...)`).
>
> (<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_reader%20%3Areader%0A%0A%20%20sig%20%7Bparams(writer%3A%20Integer).returns(Integer)%7D%0A%20%20attr_writer%20%3Awriter%0A%0A%20%20%23%20For%20attr_accessor%2C%20write%20the%20sig%20for%20the%20reader%20portion.%0A%20%20%23%20(Sorbet%20will%20use%20that%20to%20write%20the%20sig%20for%20the%20writer%20portion.)%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_accessor%20%3Aaccessor%0A%0A%20%20sig%20%7Bparams(reader%3A%20Integer%2C%20writer%3A%20Integer%2C%20accessor%3A%20Integer).void%7D%0A%20%20def%20initialize(reader%2C%20writer%2C%20accessor)%0A%20%20%20%20%40reader%20%3D%20reader%0A%20%20%20%20%40writer%20%3D%20writer%0A%20%20%20%20%40accessor%20%3D%20accessor%0A%20%20end%0Aend">→
> Full example on sorbet.run</a>)

## What's the difference between `Array` and `T::Array[…]`?

`Array` is the built-in Ruby class for arrays. On the other hand,
`T::Array[...]` is a Sorbet generic type for arrays. The `...` must always be
filled in when using it.

While Sorbet implicitly treats `Array` the same as `T::Array[T.untyped]`, Sorbet
will error when trying to use `T::Array` as a standalone type.

[→ Generics in the Standard Library](stdlib-generics.md)

## How do I accept a class object, and not an instance of a class?

[→ T.class_of](class-of.md)

## How do I write a signature for `initialize`?

When defining `initialize` for a class, we strongly encourage that you use
`.void`. This reminds people instantiating your class that they probably meant
to call `.new`, which is defined on every Ruby class. Typing the result as
`.void` means that it's not possible to do anything with the result of
`initialize`.

[→ Method Signatures](sigs.md)

## How do I override `==`? What signature should I use?

The same suggestions also apply for overriding `eql?`. `==` is typically used
for custom/structural equality operators. `eql?` is typically used for reference
equality. But from the perspective of the type system, they both have the same
signature.

Your method should accept `T.anything` and return `T::Boolean`. For more
information, see the docs for [`T.anything`](anything.md) and the blog post
[Problems typing equality in Ruby].

[Problems typing equality in Ruby]:
  https://blog.jez.io/problems-typing-ruby-equality/

If you want the implementation to branch on the type of the argument, you'll
want to use `case`/`when` (or `Module#===` directly) instead of `is_a?`.

Example using `case`/`when`:

```ruby
class A
  # ...
  attr_reader :foo

  sig { params(other: T.anything).returns(T::Boolean) }
  def ==(other)
    case other
    when A
      self.foo == other.foo
    else
      false
    end
  end
end
```

Example using [`===` directly][case subsumption]:

```ruby
class A
  # ...
  attr_reader :foo

  sig { params(other: T.anything).returns(T::Boolean) }
  def ==(other)
    (A === other) && (self.foo == other.foo)
  end
end
```

[case subsumption]: https://stackoverflow.com/questions/3422223/vs-in-ruby

## I use `T.must` a lot with arrays and hashes. Is there a way around this?

`Hash#[]` and `Array#[]` return a [nilable type](nilable-types.md) because in
Ruby, accessing a Hash or Array returns `nil` if the key does not exist or is
out-of-bounds. If you would rather raise an exception than handle `nil`, use the
`#fetch` method:

```ruby
[0, 1, 2].fetch(3) # IndexError: index 3 outside of array bounds
```

## Sigs are vague for stdlib methods that accept keyword arguments & have multiple return types

You might notice this when calling `Array#sample`, `Pathname#find`, or other
stdlib methods that accept a keyword argument and can have different return
types based on arguments:

```ruby
T.reveal_type([1, 2, 3].sample) # Revealed type: T.nilable(T.any(Integer, T::Array[Integer]))
```

The sig in Sorbet's stdlib is quite wide, since it has to cover every possible
return type. Sorbet does not have good support for this for methods that accept
keyword arguments. [#37](https://github.com/sorbet/sorbet/issues/37) is the
original report of this.
[#2248](https://github.com/sorbet/sorbet/pull/2248#issuecomment-562728417) has
an explanation of why this is hard to fix in Sorbet.

To work around this, you'll need to use `T.cast`.

```ruby
item = T.cast([1, 2, 3].sample, Integer)
T.reveal_type(item) # Revealed type: Integer
```

In some cases - for example, with complex number conversion methods in
`Kernel` - the Sorbet team has chosen to ship technically incorrect RBIs that
are much more pragmatic. See [#1144](https://github.com/sorbet/sorbet/pull/1144)
for an example. You can do the same for other cases you find annoying, but you
take on the risk of always need to call the method correctly based on your new
sig.

For example, if you are confident you'll never call `Array#sample` on an empty
array, use this [RBI](rbi.md) to not have to worry about `nil` returns.

```ruby
class Array
  extend T::Sig

  sig do
    params(arg0: Integer, random: Random::Formatter)
    .returns(T.any(Elem, T::Array[Elem]))
  end
  def sample(arg0=T.unsafe(nil), random: T.unsafe(nil)); end
end
```

Or if you never call it with an argument (you always do `[1,2,3].sample`, never
`[1,2,3].sample(2)`), use this RBI to always get an element (not an array) as
your return type:

```ruby
class Array
  extend T::Sig

  sig do
    params(arg0: Integer, random: Random::Formatter)
    .returns(Elem) # or T.nilable(Elem), to also support empty arrays
  end
  def sample(arg0=T.unsafe(nil), random: T.unsafe(nil)); end
end
```

> Overriding stdlib RBIs can make type checking less safe, since Sorbet will now
> have an incorrect understanding of how the stdlib behaves.

Another alternative is to define new methods that are stricter about arguments,
and use these in place of stdlib methods:

```ruby
class Array
  extend T::Sig

  sig { returns(Elem) } # or T.nilable(Elem) unless you're confident this is never called on empty arrays
  def sample_one
    T.cast(sample, Elem)
  end

  sig { params(n: Integer).returns(T::Array[Elem]) }
  def sample_n(n)
    T.cast(sample(n), T::Array[Elem])
  end
end
```

## Why is `super` untyped, even when the parent method has a `sig`?

By default, Sorbet type checks all calls to `super` that it can, but there are
still cases where it can't statically check calls to `super`.

See below.

Note that like with most parts of Sorbet, support for typing `super` is expected
to improve over time, which might introduce new type errors upon upgrading to a
newer Sorbet version.

## When are calls to `super` typed, and when are they untyped?

Over time, we have improved support for `super` in certain circumstances. In
particular, if all of these things are true, Sorbet will type check the call to
`super`:

- The usage of `super` must be in a method defined in a `class`, not a module.

  (Why? → Inside a module, the method that `super` dispatches to [is different
  each time][module_super] the module is mixed into a different class. This
  means there's not necessarily a single way to analyze a call to `super`
  statically.)

- The usage of `super` must not be inside a Ruby block (like `do ... end`)

  (Why? → Sorbet doesn't always know whether the block is being passed to
  `define_method`, or otherwise [executed in a context different from the
  enclosing context][define_method_super].)

[module_super]:
  https://sorbet.run/#%23%20typed%3A%20true%0Amodule%20MyModule%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20foo%0A%20%20%20%20%23%20Can't%20know%20super%20until%20we%20know%20which%20module%20we're%20mixed%20into%0A%20%20%20%20res%20%3D%20super%0A%20%20%20%20T.reveal_type%28res%29%0A%20%20%20%20res%0A%20%20end%0Aend%0A%0Amodule%20ParentModule1%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20foo%0A%20%20%20%200%0A%20%20end%0Aend%0A%0Amodule%20ParentModule2%0A%20%20extend%20T%3A%3ASig%0A%20%20%0A%20%20sig%20%7Breturns%28String%29%7D%0A%20%20def%20foo%0A%20%20%20%20''%0A%20%20end%0Aend%0A%0Aclass%20MyClass1%0A%20%20include%20ParentModule1%0A%20%20include%20MyModule%0Aend%0A%0Aclass%20MyClass2%0A%20%20include%20ParentModule2%0A%20%20include%20MyModule%0Aend%0A%0AMyClass1.new.foo%0AMyClass2.new.foo%0A
[define_method_super]:
  https://sorbet.run/#%23%20typed%3A%20strict%0A%0Aclass%20Parent%0A%20%20extend%20T%3A%3ASig%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20self.foo%0A%20%20%20%200%0A%20%20end%0A%0A%20%20sig%20%7Breturns%28String%29%7D%0A%20%20def%20self.bar%0A%20%20%20%20''%0A%20%20end%0Aend%0A%0Aclass%20Child%20%3C%20Parent%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20self.foo%0A%20%20%20%20define_method%28%3Abar%29%20do%0A%20%20%20%20%20%20x%20%3D%20super%0A%20%20%20%20%20%20T.reveal_type%28x%29%0A%20%20%20%20%20%20%23%20-%3E%20should%20reveal%20String%2C%20but%20Sorbet%20doesn't%20know%20that%0A%20%20%20%20end%0A%0A%20%20%20%200%0A%20%20end%0Aend

To opt out of type checking `super`, pass the `--typed-super=false` command line
flag to Sorbet.

## How can I fix type errors that arise from `super`?

1.  "Method does not exist" type errors can happen when Sorbet does not see that
    a method with the same name actually exists on an ancestor of the current
    class. Double check whether such a method actually exist. If it does,
    metaprogramming is likely hiding that definition from Sorbet. Use
    [RBI files](rbi.md) to ensure that Sorbet sees the parent's definition.

1.  Usually type errors from `super` arise because of incompatible overrides,
    [like this][super_incompatible]. Sorbet only does
    [Override Checking](override-checking.md) when the parent method is declared
    with `overridable` or `abstract`, which means that incompatible overrides
    can creep into a codebase.

    To fix these errors, correct the signature so that the child signature is a
    [valid override](override-checking.md#a-note-on-variance).

1.  Sometimes the type errors happen even when the child is a valid override of
    the parent, because the child either promises to accept a wider input or
    return a narrower output. In these cases, before and/or after calling
    `super`, the child method will need to handle the cases that the parent
    method's type is unable to handle. (Or, if possible: adjust the parent's
    signature to match the child's signature.)

1.  All `initialize` calls in Sorbet are typed as returning `.void`, regardless
    of what they actually return. Usually, calls to `initialize` simply return
    `self`. So instead of doing this:

    ```ruby
    def initialize
      super.foo
    end
    ```

    do this:

    ```ruby
    def initialize
      super
      self.foo
    end
    ```

1.  Unlike method calls, it's **not possible** to do something like
    `T.unsafe(self).super` to silence errors from a single usage of `super`
    (`super` is a keyword in Ruby, not a method call).

    This means that there is not a way to silence errors from a single usage of
    `super`. The best you can do is silence all errors inside a method using
    `T.bind`:

    ```ruby
    def example
      T.bind(self, T.untyped) # <------
      super
    end
    ```

    See the docs for [T.bind](procs.md#casting-the-self-type-with-tbind). Note:
    not only does this opt out of typechecking calls to `super`, but it also
    opts out of type checking for all method calls on `self` in that method
    body.

1.  To silence all type errors from super, use the `--typed-super=false` flag.
    This opts out of all `super` type checking.

[super_incompatible]:
  https://sorbet.run/#%23%20typed%3A%20strict%0A%0Aclass%20Parent%0A%20%20extend%20T%3A%3ASig%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20self.foo%0A%20%20%20%200%0A%20%20end%0Aend%0A%0Aclass%20Child%20%3C%20Parent%0A%20%20sig%20%7Breturns%28String%29%7D%0A%20%20def%20self.foo%0A%20%20%20%20super%0A%20%20end%0Aend

## Why does `T.untyped && T::Boolean` have type `T.nilable(T::Boolean)`?

**The short answer**: because `T.untyped` includes `nil`, and `nil && false` is
`nil`. Therefore, the whole expression's type is possibly nilable.

**The long answer**: given a snippet like this

```ruby
sig { params(x: T.untyped, y: T::Boolean).void }
def example(x, y)
  res = x && y
  T.reveal_type(res) # => T.nilable(T::Boolean)
end
```

The revealed type is [`T.nilable`](nilable-types.md) despite neither `x` nor `y`
being `T.nilable`. Ruby's `&&` operator is short-circuiting, and `nil` is falsy:

```ruby
x = nil && anything_else()
p(x) # => nil
```

This `&&` expression evaluates to `nil` without evaluating `anything_else()`.

So thinking about our `res = x && y` expression:

1.  If `x` is `nil`, the whole expression is `nil` and the type must be at least
    [`NilClass`](class-types.md#nil).
1.  If `x` is `false`, the whole expression's type must be at least
    [`FalseClass`](class-types.md#booleans).
1.  Otherwise, `x` is truthy (only `nil` and `false` are falsy in Ruby), so the
    expression evaluates to the value of `y`, assuming it's type. In this case,
    `T::Boolean`.

Therefore, the type is `T.any(NilClass, FalseClass, T::Boolean)` which
simplifies to `T.nilable(T::Boolean)`.

## Does Sorbet work with Rake and Rakefiles?

Kind of, with some effort. Rake monkey patches the global `main` object (i.e.,
top-level code) to extend their DSL, which Sorbet cannot understand:

```ruby
# -- from lib/rake/dsl_definition.rb --

...

# Extend the main object with the DSL commands. This allows top-level
# calls to task, etc. to work from a Rakefile without polluting the
# object inheritance tree.
self.extend Rake::DSL
```

→
[lib/rake/dsl_definition.rb](https://github.com/ruby/rake/blob/80e00e2d59ea5b230f2f0416c387c0b57184f1ff/lib/rake/dsl_definition.rb#L192-L195)

Sorbet cannot model that a single instance of an object (in this case `main`)
has a different inheritance hierarchy than that instance's class (in this case
`Object`).

To get around this, factor out all tasks defined the `Rakefile` that should be
typechecked into an explicit class in a separate file, something like this:

```ruby
# -- my_rake_tasks.rb --

# (1) Make a proper class inside a file with a *.rb extension
class MyRakeTasks
  # (2) Explicitly extend Rake::DSL in this class
  extend Rake::DSL

  # (3) Define tasks like normal:
  task :test do
    puts 'Testing...'
  end

  # ... more tasks ...
end

# -- Rakefile --

# (4) Require that file from the Rakefile
require_relative './my_rake_tasks'
```

For more information, see
[this StackOverflow question](https://stackoverflow.com/a/60556206/1015863).

## How do I upgrade Sorbet?

Sorbet has not reached version 1.0 yet. As such, it will make breaking changes
constantly, without warning.

To upgrade a patch level (e.g., from 0.4.4314 to 0.4.4358):

```
bundle update sorbet sorbet-runtime
# also update plugins, if any

# Optional: Suggest new, stronger sigils (per-file strictness
# levels) when possible. Currently, the suggestion process is
# fallible, and may suggest downgrading when it's not necessary.
bundle exec srb rbi suggest-typed
```

## What platforms does Sorbet support?

The `sorbet-runtime` gem is currently only tested on Ruby 2.6 and Ruby 2.7. It
is known to not support Ruby 2.4. Feel free to report runtime issues for any
current or future Ruby version.

The `sorbet-static` gem is known to support Ruby 2.4, Ruby 2.5, Ruby 2.6, and
Ruby 2.7 to a minimum level (i.e., it can at least parse syntax introduced in
those versions). Some language features are typed more strictly than others
(generally, language features in newer Ruby versions have looser type support).
This is not by design, just by convenience. Feel free to open feature requests
that various (new or old) language features be typed more strictly.

Sorbet bundles [RBI files](rbi.md) for the standard library. In Ruby the
standard library changes with the Ruby version being used, but Sorbet only ships
one set of RBI definitions for the standard library. In particular, Sorbet's RBI
files for the standard library might reflect classes, methods, or APIs that are
only available in a version of Ruby newer than the one used to run a given
project. You will have to rely on (runtime) test suites to verify that your
project does not use new standard library APIs with an old Ruby version.

The `sorbet-static` gem is only tested on macOS 10.14 (Mojave) and Ubuntu 18
(Bionic Beaver). There is currently no Windows support. We expect
`sorbet-static` to work as far back as macOS 10.10 (Yosemite), as far forward as
macOS 11.0 Big Sur, and on most Linux distributions using `glibc`.

We do not test nor publish prebuilt binaries for macOS on Apple Silicon. We have
reports that it doesn't work, but no one on the Sorbet team has access to Apple
Silicon-based macOS machines, so we have been unable to diagnose nor fix any
problems. If you are interested in working on this, feel free to reach out in
the #internals channel on our [Sorbet Slack](/slack).

The `sorbet` gem has runtime dependencies on `git` and `bash`.

Combined, these points mean that if you are using one of the official minimal
Ruby Docker images (which are based on Apline Linux), you will need to install
some support libraries:

```Dockerfile
FROM ruby:2.6-alpine

RUN apk add --no-cache --update \
    git \
    bash \
    ca-certificates \
    wget

ENV GLIBC_RELEASE_VERSION 2.30-r0
RUN wget -nv -O /etc/apk/keys/sgerrand.rsa.pub https://alpine-pkgs.sgerrand.com/sgerrand.rsa.pub && \
    wget -nv https://github.com/sgerrand/alpine-pkg-glibc/releases/download/${GLIBC_RELEASE_VERSION}/glibc-${GLIBC_RELEASE_VERSION}.apk && \
    apk add glibc-${GLIBC_RELEASE_VERSION}.apk && \
    rm /etc/apk/keys/sgerrand.rsa.pub && \
    rm glibc-${GLIBC_RELEASE_VERSION}.apk
```

## Does Sorbet support ActiveRecord (and Rails?)

Sorbet doesn't support Rails, but
[Tapioca can generate RBI files for it](https://github.com/Shopify/tapioca#generating-rbi-files-for-rails-and-other-dsls).

Also see the [Community] page for more community-maintained projects!

[community]: /en/community

## Can I convert from YARD docs to Sorbet signatures?

[Sord] is a community-maintained project which can generate Sorbet RBI files
from YARD docs.

Also see the [Community] page for more community-maintained projects!

[sord]: https://github.com/AaronC81/sord

## When Ruby 3 gets types, what will the migration plan look like?

The Sorbet team is actively involved in the Ruby 3 working group for static
typing. There are some things we know and something we don't know about Ruby 3.

Ruby 3 plans to ship type annotations for the standard library. These type
annotations for the standard library will live in separate Ruby Signature (RBS)
files, with the `*.rbs` extension. The exact syntax is not yet finalized. When
the syntax is finalized, Sorbet intends to ingest both RBS and RBI formats, so
that users can choose their favorite.

Ruby 3 has no plans to change Ruby's syntax. To have type annotations for
methods live in the same place as the method definition, the only option will be
to continue using Sorbet's [method signatures](sigs.md). As such, the Sorbet
docs will always use RBI syntax in examples, because the syntax is the same for
signatures both within a Ruby file and in external [RBI files](rbi.md).

Ruby 3 has no plans to ship a type checker for RBS annotations. Instead, Ruby 3
plans to ship a type profiler, which will attempt to guess signatures for code
without signatures. The only way to get type checking will be to use third party
tools, like Sorbet.

Ruby 3 plans to ship no specification for what the type annotations mean. Each
third party type checker and the Ruby 3 type profiler will be allowed to ascribe
their own meanings to individual type annotations. When there are ambiguities or
constructs that one tool doesn't understand, it should fall back to `T.untyped`
(or the equivalent in whatever RBS syntax decides to use for
[this construct](untyped.md)).

Ruby 3 plans to seed the initial type annotations for the standard library from
Sorbet's extensive existing type annotations for the standard library. Sorbet
already has great type annotations for the standard library in the form of
[RBI files](rbi.md) which are used to type check millions of lines of production
Ruby code every day.

From all of this, we have every reason to believe that users of Sorbet will have
a smooth transition to Ruby 3:

- You will be able to either keep using Sorbet's RBI syntax or switch to using
  RBS syntax.
- The type definitions for the standard library will mean the same (because they
  will have come from Sorbet!) but have a different syntax.
- For inline type annotations with Ruby 3, you will have to continue using
  Sorbet's `sig` syntax, no different from today.

For more information, watch [this section](https://youtu.be/2g9R7PUCEXo?t=2022)
from Matz's RubyConf 2019 keynote, which talks about his plans for typing in
Ruby 3.

## Can I use Sorbet for duck typed code?

No. You can use an [interface](abstract.md) instead, or `T.untyped` if you do
not control all of the code.

Duck typing (or, more formally, Structural typing) specifies types by their
structure. For example, Rack middleware accepts any object that has a `call`
method which takes one argument and returns a tuple representing an HTTP
response.

Sorbet does not support duck typing either for static analysis or runtime
checking.

## How do I see errors from a single file, not the whole project?

Fundamentally, Sorbet typechecks a single project at a time. Unlike other
languages and compilers, it does not process files in a codebase one file at a
time. This is largely due to the fact that Ruby does not have any sort of
file-level import mechanism—in Ruby, code can usually be defined anywhere and
used from anywhere else—and this places constraints on how Sorbet must be
implemented.

Because Sorbet typechecks a single project at a time, it does not allow
filtering the list of errors to a single file. Sometimes though, the number of
errors Sorbet reports can be overwhelming, especially when adopting Sorbet in a
new codebase. In these cases, the best way to proceed is as follows:

1.  Put every file in the codebase at `# typed: false` (see
    [Strictness Levels](static.md#file-level-granularity-strictness-levels)).
    Note that the default strictness level when there is no explicit `# typed:`
    comment in a file is `# typed: false`. The `--typed=false` command line flag
    can be used to forcibly override every file's strictness level to
    `# typed: false`, regardless of what's written in the file.

    Forcing every file to `# typed: false` will silence all but the most
    critical Sorbet errors throughout the project.

1.  Proceed to fix these errors. If there are still an overwhelming number of
    errors, tackle the errors that are reported earlier first. Sorbet's design
    means that errors discovered early in the early phases of typechecking can
    cause redundant errors in later phases.

1.  Once there are no errors in any files at `# typed: false`, proceed to
    upgrade individual files to `# typed: true`. Only new errors will be
    reported in these `# typed: true` files, and not in any other files.
    Repeatedly upgrade as many individual files as is preferred. Note that many
    Sorbet codebases start off with all files at `# typed: false` and gradually
    (usually organically) shrink the number of `# typed: false` files over time.

If for some reason it's still imperative to limit the Sorbet error output to
only those errors coming from a single file and the steps above are not
acceptable, we recommend post processing the errors reported by Sorbet with
tools like `grep`.

There are also third-party tools that offer the ability to sort and filter
Sorbet's errors, like [spoom].

[spoom]: https://github.com/Shopify/spoom#errors-sorting-and-filtering
