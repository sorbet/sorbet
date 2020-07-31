---
id: ruby-3-rbs-sorbet
title: Types in Ruby 3, RBS, and Sorbet
author: Jake Zimmerman
authorURL: https://twitter.com/jez_io
authorImageURL: https://avatars0.githubusercontent.com/u/5544532?s=460&v=4
---

Yesterday Square [posted an article][introducing-rbs] to their blog introducing
[RBS][rbs] (Ruby Signature), a type syntax format for Ruby 3.

We'd like to take a second to speak to how RBS relates to Sorbet. The short
version: Sorbet will happily incorporate RBS as a way to specify type
annotations, in addition to the existing syntax Sorbet supports. Stripe still
has a very strong commitment to Sorbet's continued progress and success. While
the Ruby core team has been working on syntax, we've been working on features
that build on top of that syntax.

With that in mind, I'd love to start a discussion of some of the finer points of
what this announcement means for Ruby, and for Sorbet.

[introducing-rbs]:
  https://developer.squareup.com/blog/the-state-of-ruby-3-typing/
[rbs]: https://github.com/ruby/rbs

<!--truncate-->

## RBI? RBS?

As [Matz announced at RubyConf 2019][rubyconf-2019], Ruby 3 plans to ship type
annotations for the standard library in a particular format. We've been meeting
with Matz and the Ruby core team to provide input on our experience and learn
from them how they're thinking about types so that Sorbet wil be ready. We're
[committed][faq-migration] to supporting Ruby 3's type syntax.

[rubyconf-2019]: https://youtu.be/2g9R7PUCEXo?t=2076
[faq-migration]:
  https://sorbet.org/docs/faq#when-ruby-3-gets-types-what-will-the-migration-plan-look-like

In the mean time, we've kept busy. While the Ruby core team has been working on
the RBS syntax over the past year, the Sorbet team has delivered tons of other
features. A sampling of features that didn't exist a year ago:

- Go to Definition (Aug 2019)
- [Exhaustiveness checking](/docs/exhaustiveness) (Aug 2019)
- [Typed enums](/docs/tenum) (Nov 2019)
- Autocompletion (Nov 2019)
- Step-function improvements in IDE speed (Feb 2020, June 2020)

These features make Sorbet users more productive, empowering them to do more
with Sorbet. We'll happily incorporate any other syntax the Ruby core team wants
to build.

## Wait, a wholly separate file?

RBS type signatures are placed in a separate file. While Sorbet also offers
inline syntax (more later), we believe that supporting type signatures in a
separate file is necessary. Consider: most of the Ruby standard library is
implemented in C for performance (e.g., all `Array` and `Hash` functions, and
many others). There must be a way to ascribe types to these internally defined
classes and methods.

Providing types for the standard library is incredibly important! In completely
untyped, unannotated Ruby codebases, people who try out Sorbet for the first
time find that about 25% of call sites already have static type coverage. Why?
Everyday Ruby code uses the standard library abundantly, and Sorbet includes
type definitions for the standard library out of the box!

Additionally, there will always be libraries that prefer not using type
annotations. To integrate an untyped library into a typed Ruby codebase, there
must be a place for these types to live outside of that project. In fact, we
already have the [sorbet-typed] repo for this purpose.

[sorbet-typed]: https://github.com/sorbet/sorbet-typed

Thus, both Sorbet and Ruby 3 support type annotations in separate files (via RBI
files and RBS files, respectively).

## What about inline type annotations?

There are people using Ruby who still prefer type annotations to live in the
code itself. We've built Sorbet to cater to both groups: those who **like and
don't like** type annotations benefit from Sorbet.

How? Sorbet implements a [gradual type system]. Without any type annotations,
Sorbet will do its best to understand and offer feedback on your code. With type
annotations exclusively in separate files, Sorbet will understand more and offer
better feedback. With inline type annotations, people can tell Sorbet every
little detail about their code so Sorbet can offer incredible feedback. For
these people, reading the types is as valuable as reading the code itself.

[gradual type system]: /docs/gradual

Sorbet has always provided syntax for inline type annotations. Annotations
aren't required by Sorbet—they're there for the people who want to empower
Sorbet to help them even more. Whether you love type annotations or not, Sorbet
still provides value.

## Why are inline type annotations useful?

As we mentioned above, for those teams and projects who really want static
typing, inline type annotations become essential. Types carry intent, and there
are frequently places where inferring the intent could be ambiguous. Consider
this snippet:

```ruby
xs = [1, 2, 3]
xs << nil

# what's the type of xs? 🤔
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Axs%20%3D%20%5B1%2C%202%2C%203%5D%0Axs%20%3C%3C%20nil)

A type checker could infer one of two types here:

1. either the programmer meant for `xs` to be an array of `Integer`'s and
   `nil`'s (i.e., `T::Array[T.nilable(Integer)]` in Sorbet's syntax), or
2. the programmer made a mistake, and didn't intend to allow `nil`'s in `xs`
   (i.e., it was intended to be a `T::Array[Integer]`, and this code should
   report a static error).

Any static checker must assume one of these outcomes, but there are times when
either might make sense. Explicit annotations resolve these ambiguities. In this
case, Sorbet assumes (2) by default, but other type systems do other things. For
example, Flow [assumes (1) by default][flow-example].

[flow-example]:
  https://flow.org/try/#0MYewdgzgLgBAHhGBeGBtAjAGhgJmwZgF0BuAKAQDoAHAVwgAsAKMGgG1YEpig

Here's how to use an inline annotation in Sorbet to explicitly declare that (1)
is intended:

```ruby
xs = T.let([1, 2, 3], T::Array[T.nilable(Integer)])
xs << nil

# the type of xs is unambiguous 👌
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Axs%20%3D%20T.let(%5B1%2C%202%2C%203%5D%2C%20T%3A%3AArray%5BT.nilable(Integer)%5D)%0Axs%20%3C%3C%20nil">→
View on sorbet.run</a>

## Inline type annotations must be Ruby syntax

For the time being, Matz and the Ruby core team want to experiment with type
annotations without changing Ruby syntax, because that would require everyone to
upgrade to a specific Ruby version to benefit. Because Sorbet values inline type
annotations, we embedded a type annotation language in Ruby with no syntax
changes needed:

```ruby
extend T::Sig

sig {params(strings: T::Array[String]).returns(Integer)}
def count_letters(strings)
  strings.map(&:length).sum
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(strings%3A%20T%3A%3AArray%5BString%5D).returns(Integer)%7D%0Adef%20count_letters(strings)%0A%20%20strings.map(%26%3Alength).sum%0Aend">→
View on sorbet.run</a>

Sorbet's syntax is 100% valid Ruby, which has tons of benefits!

- Syntax highlighting for type annotations already works in 100% of Ruby
  editors.
- There's no transpiler step required—Ruby code with Sorbet type annotations
  runs directly.
- Every RuboCop rule ever written works with Sorbet type annotations.
- Any Ruby IDE with Go to Definition already has Go to Definition on type
  annotations.
- Sorbet's RBI files are just Ruby code with empty method bodies, reusing the
  inline syntax.
- It's backwards compatible with all supported versions of Ruby.

The obvious downside is that there are prettier inline type annotation syntaxes
that are not valid Ruby. The Ruby grammar is not so complicated that it couldn't
be changed to support type annotations. But again, this syntax does so well
**because** it doesn't fracture the Ruby community with incompatible syntax
changes.

## Sorbet is committed to improving

At the end of the day, it's not a choice between Ruby 3 or Sorbet—you can have
both at the same time. We love that the Ruby core team is bringing types to
Ruby, and we're happy to incorporate their work into Sorbet. We plan to give
back too: it's likely that the initial release of RBS files for the Ruby
standard library will be created by converting Sorbet's already extensive
standard library annotations into the RBS format.

At the same time, we're hard at work improving Sorbet. Stripe has millions of
lines of Ruby code and that number is only growing. We're nearing our [second
nine] of percentage of files at `# typed: true` or above—which is to say, those
millions of lines of Ruby use a lot of types. We have hundreds of engineers
writing Ruby and using our Sorbet-powered IDE every day.

[second nine]:
  https://en.wikipedia.org/wiki/High_availability#Percentage_calculation

Sorbet type checks these millions of lines of Ruby code in seconds, helps
prevent countless production incidents, and helps new Stripe engineers spin up
fast. And it does all this today! If you want to try it out, check out the docs:

[→ Adopting Sorbet in an Existing Codebase](https://sorbet.org/docs/adopting)

or play around with small Sorbet examples online:

[→ Sorbet playground](https://sorbet.run)

Sorbet already has extensive type annotations for the Ruby standard library,
thanks in large part to the nearly 200 contributors to Sorbet that you can find
[on GitHub][contributors], the vast majority of whom come to Sorbet from outside
of Stripe and the Sorbet team.

[contributors]: https://github.com/sorbet/sorbet/graphs/contributors

We're happy that the Ruby core team is focused on continually improving Ruby,
because we are too. ❤️

— Jake "jez" Zimmerman, on behalf of the Sorbet team

_Thanks to Dmitry Petrashko, James Iry, Trevor Elliot, and Soutaro Matsumoto for
reading early drafts of this post._
