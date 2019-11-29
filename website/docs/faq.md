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

## What's the difference between `T.let`, `T.cast`, and `T.unsafe`?

[→ Type Assertions](type-assertions.md)

## What's the type signature for a method with no return?

```ruby
sig {void}
```

[→ Method Signatures](sigs.md)

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

Your method should accept `BasicObject` and return `T::Boolean`.

Unfortunately, not all `BasicObject`s have `is_a?`, so we have to do one extra
step in our `==` function: check whether `Object === other`. (In Ruby, `==` and
`===` are completely unrelated. The latter has to do with [case subsumption]).
The idiomatic way to write `Object === other` in Ruby is to use `case`:

```ruby
case other
when Object
  # ...
end
```

[case subsumption]: https://stackoverflow.com/questions/3422223/vs-in-ruby

Here's a complete example that uses `case` to implement `==`:

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20IntBox%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_reader%20%3Aval%0A%20%20%0A%20%20sig%20%7Bparams(val%3A%20Integer).void%7D%0A%20%20def%20initialize(val)%0A%20%20%20%20%40val%20%3D%20val%0A%20%20end%0A%20%20%0A%20%20sig%20%7Bparams(other%3A%20BasicObject).returns(T::Boolean)%7D%0A%20%20def%20%3D%3D(other)%0A%20%20%20%20case%20other%0A%20%20%20%20when%20IntBox%0A%20%20%20%20%20%20other.val%20%3D%3D%20val%0A%20%20%20%20else%0A%20%20%20%20%20%20false%0A%20%20%20%20end%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

## I use `T.must` a lot with arrays and hashes. Is there a way around this?

`Hash#[]` and `Array#[]` return a [nilable type](nilable-types.md) because in
Ruby, accessing a Hash or Array returns `nil` if the key does not exist or is
out-of-bounds. If you would rather raise an exception than handle `nil`, use the
`#fetch` method:

```ruby
[0, 1, 2].fetch(3) # IndexError: index 3 outside of array bounds
```

## How do I upgrade Sorbet?

Sorbet has not reached version 1.0 yet. As such, it will make breaking changes
constantly, without warning.

To upgrade a patch level (e.g., from 0.4.4314 to 0.4.4358):

```
bundle update sorbet sorbet-runtime
# also update plugins like sorbet-rails, if any
bundle exec srb init

# For plugins like sorbet-rails, see their docs, eg.
https://github.com/chanzuckerberg/sorbet-rails#initial-setup

# Optional: Suggest new, stronger sigils (per-file strictness
# levels) when possible. Currently, the suggestion process is
# fallible, and may suggest downgrading when it's not necessary.
bundle exec srb rbi suggest-typed
```

## What platforms does Sorbet support?

The `sorbet` and `sorbet-runtime` gems are currently only tested on Ruby 2.4. We
expect it to work on Ruby 2.3 through 2.6.

The static check is only tested on macOS 10.14 (Mojave) and Ubuntu 18 (Bionic
Beaver). We expect it to work on macOS 10.10 (Yosemite) and most Linux
distributions where `glibc`, `git` and `bash` are present. We use static linking
on both platforms, so it should not depend on system libraries.

If you are using one of the official minimal Ruby Docker images you will need to
install the extra dependencies yourself:

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

There is currently no Windows support.

## Does Sorbet support ActiveRecord (and Rails?)

[sorbet-rails] is a community-maintained project which can help generate RBI
files for certain Rails constructs.

Also see the [Community] page for more community-maintained projects!

[sorbet-rails]: https://github.com/chanzuckerberg/sorbet-rails
[community]: /en/community

## Can I convert from YARD docs to Sorbet signatures?

[Sord] is a community-maintained project which can generate Sorbet RBI files
from YARD docs.

Also see the [Community] page for more community-maintained projects!

[sord]: https://github.com/AaronC81/sord

## Can Sorbet produce statistics?

Yes, you can use options like `--metrics-file` to produce statistics. For
example, check out
[sorbet-progress](https://github.com/jaredbeck/sorbet-progress) which uses those
statistics to keep track of your progress adopting sorbet in big codebases.
