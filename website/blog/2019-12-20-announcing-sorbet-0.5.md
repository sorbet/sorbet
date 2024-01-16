---
id: open-sourcing-sorbet
title: Announcing Sorbet 0.5
author: Jake Zimmerman
authorURL: https://twitter.com/jez_io
authorImageURL: https://avatars0.githubusercontent.com/u/5544532?s=460&v=4
---

Today we're excited to celebrate six months since Sorbet's open source release!

Sorbet is a fast, powerful type checker for Ruby developed by Stripe and an
ever-growing community of contributors. You can [try it online][sorbet.run] or
[set it up in your project][adopting] today. Sorbet gradually integrates into
existing Ruby projects. With Sorbet, people writing Ruby gain more confidence in
their changes and get faster feedback while iterating.

At this milestone, we'd like to take a look back on what's happened since the
first public release of Sorbet, and what's coming in the future.

[sorbet.run]: https://sorbet.run
[adopting]: https://sorbet.org/docs/adopting

<!--truncate-->

## Community involvement

The most exciting development since open-sourcing Sorbet has been watching the
community grow. After more than two years developing Sorbet, we've known that
teams within Stripe find Sorbet valuable. But outside of Stripe, we couldn't be
sure until other people tried it for themselves and told us how it went.

Six months later, it's clear Sorbet is finding success outside of Stripe too.
Rather than take our word for it, you can watch these talks by Sorbet users:

- Ufuk Kayserilioglu from Shopify spoke about [Adopting Sorbet at
  Scale][shopify-talk] at RubyConf 2019
- Harry Doan hosted an event at the Chan Zuckerberg Initiative to talk about
  [Using Sorbet with Rails][czi-talk]

On top of that, we've had the pleasure to chat with users of Sorbet every day in
our [Sorbet Slack community][slack]. These conversations let us know what works,
what can be improved, and what use cases people are finding for Sorbet. Many of
the features we've implemented since open-sourcing Sorbet were direct asks from
users! (More on those features in the sections below.)

If you're using Sorbet already or thinking about giving it a try, you can find
us here:

[→ Join the Sorbet Slack community][slack]

The Sorbet team staffs a rotation with at least one person reading and
responding to new messages, and questions frequently get answered by our
friendly community.

## Community contributions

We've also seen this community actively contribute back:

- The main Sorbet repo has more than **140 contributors**.

- These contributors have collectively landed **nearly 1000 pull requests**,
  with about 25% of those contributions coming from our open-source community.

- Of these contributions, 25% of them add or improve type definitions for the
  standard library or third-party gems in the form of [RBI files][rbi-files].

  RBI files for the standard library ship with Sorbet, while RBI files for
  third-party gems are hosted in a central repository called [sorbet-typed].
  Sorbet automatically fetches these definitions when they exist, or creates
  untyped skeleton definitions if they don't.

We couldn't have gotten this far without such a great community, and we look
forward to what happens next. Thanks so much!

## New features

As you might imagine, those thousand pull requests have changed **a lot** of
things in Sorbet. Let's take a look at some of the most exciting new features.

### Exhaustiveness checking

Sorbet has always supported [union types][union-types], which declare that a
value can be one of a finite set of types:

```ruby
# (1) T.any(Integer, String) is a union type
sig {params(x: T.any(Integer, String)).void}
def foo(x)
  case x
  when Integer then # (2) x must be an Integer here
  when String  then # (3) x must be a String here
  end
end
```

Union types are a must-have for type checking real-world Ruby code bases. Sorbet
supports union types by tracking the way control flows through a program to
update its knowledge of what type each variable has at different points. In the
`case` statement above, Sorbet knows that within each `when` branch, the
variable `x` has a more specific type than it does outside the `case` statement.

In the last six months, Sorbet gained the ability to guarantee that **all cases
must be handled** (exhaustively). For example:

```ruby
sig {params(x: T.any(Integer, String)).void}
def foo(x)
  case x
  when Integer then # x ...
  # (1) Whoops! The case for String isn't handled.
  else
    # (2) Ask Sorbet to guarantee that all cases are handled:
    T.absurd(x) # error: the type `String` wasn't handled
  end
end
```

In this example, we've forgotten to handle the case when `x` is a `String`, but
in the `else` branch there's a call to `T.absurd(x)`. This line asks Sorbet to
report an error when control flow could reach that point, which happens when one
or more cases aren't handled.

Exhaustiveness checks are a powerful feature. They make code easier to change,
because when adding or removing a case Sorbet will report all places that need
to be updated to handle that case. You can learn more about exhaustiveness
checking in [our documentation][exhaustiveness-docs].

### Typed enums

After adding [exhaustiveness checks][exhaustiveness-docs], we built more
features to make them easier to use and more powerful. First, we built support
for [typed enums][enums-docs]:

```ruby
class Suit < T::Enum
  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end
end
```

This declares an enum representing the suits of a standard deck of playing
cards. As seen in the snippet, enums in Sorbet are normal Ruby classes: they're
created by subclassing `T::Enum`, and individual values are instances of that
class, created by calling `new`. Because of this, **enums in Sorbet are
naturally type-safe**: one enum value cannot be used where some other enum is
expected, and vice versa. (By comparison, existing Ruby code often uses symbols
like `:spades` or `:hearts` for enums, but all symbols are interchangeable at
the type-level, so they provide no type safety.)

Enums work hand-in-hand with exhaustiveness checks by design:

```ruby
sig {params(suit: Suit).void}
def color_of_suit(suit)
  case suit
  when Suit::Spades, Suit::Clubs then puts 'Black!'
  when Suit::Hearts, Suit::Diamonds then puts 'Red!'
  else T.absurd(suit) # <- guarantees that we handled all suits
end
```

Curious about enums? [Read more in the documentation][enums-docs].

### Sealed classes and modules

In addition to typed enums, we also built [sealed classes and
modules][sealed-docs] to power up exhaustiveness checks:

```ruby
module Result
  include T::Helpers
  sealed!
end

class Found < T::Struct
  include Result
  prop :id, String
end

class NotFound < T::Struct
  include Result
  prop :error_message, String
end
```

Sealing a class prevents it from being subclassed in other files. (Sealed
modules are the same, but with `include` / `extend` instead of subclassing.) By
restricting where inheritance happens, Sorbet can treat sealed classes and
modules **as if they were union types** for the sake of exhaustiveness. For
example:

```ruby
sig {params(result: Result).void}
def handle_result(result)
  case result
  when Found then puts "Found object with ID #{result.id}"
  # (uncommenting fixes the error)
  # when NotFound then puts found.error_message
  else T.absurd(result) # error: the type `NotFound` wasn't handled
  end
end
```

Because the sealed module `Result` behaves almost identically to a union type,
when we call `T.absurd(result)` in the `else` branch of this snippet Sorbet can
tell us that we forgot to handle the `NotFound` case.

Sealed classes are powerful, but maybe a little bit confusing at first glance!
Be sure to [check out the documentation][sealed-docs] for more examples and
in-depth explanations. Many teams at Stripe have met with great success using
sealed classes to simplify their code, especially around error handling.

### Easier `typed: strict` adoption

Our experience has shown us that after the initial adoption period, there comes
a point when people working with Sorbet switch from relative skepticism about
types to earnest adoption. When this switch happens, one of the easiest ways to
spread its usage is to upgrade files to `typed: strict`.

Sorbet has an [assortment of typedness levels][typedness-docs]. `typed: strict`
is the one where Sorbet requires type annotations for methods, instance
variables, and constants. (Types for local variables are always inferred.) When
these definitions lack explicit types, Sorbet implicitly treats them as
`T.untyped`, which is a sort of "[anything goes][untyped-docs]" type. So
`typed: strict` is a way to guarantee that all new code is explicitly annotated,
while the types are fresh in the author's mind.

In the months since open-sourcing, we've heard from our users that they wanted
an easier adoption path for `typed: strict`. Previously this involved writing a
lot of type annotations, many of which were annoying to write or cluttered the
code. Multiple improvements have made adoption easier, which we'll call out
individually across the next three sections.

### Suggesting types for constants

The first feature we built to ease `typed: strict` adoption is to automatically
suggest type annotations for constants. Like we mentioned in the last section,
`typed: strict` requires type annotations for constants, and in lower typedness
levels, constants lacking annotations are implicitly treated as untyped. For
example, this constant is untyped and needs a type annotation:

```ruby
# typed: strict
A = [1, 2, 3] # error: Constants must have type annotations
```

We changed Sorbet so that while it still reports the error, it will also suggest
a potential type annotation:

```console
editor.rb:2: Constants must have type annotations with `T.let` when specifying `# typed: strict` https://srb.help/7027
     2 |A = [1, 2, 3]
            ^^^^^^^^^
  Autocorrect: Use `-a` to autocorrect
    editor.rb:2: Replace with `T.let([1, 2, 3], T::Array[Integer])`
     2 |A = [1, 2, 3]
            ^^^^^^^^^
Errors: 1
```

Note the `Autocorrect:` suggestion: Sorbet guessed what type annotation would
work and presented it to the user. We can even ask Sorbet to edit the file in
place to accept this suggestion by re-running sorbet with the `-a` /
`--autocorrect` flag, resulting in this file:

```ruby
# typed: strict
A = T.let([1, 2, 3], T::Array[Integer])
```

Automation like this is the primary way the Sorbet team and other teams at
Stripe have driven such high adoption in such a short time. For example, Sorbet
has had suggested annotations [for methods][suggest-sig-example] (not constants)
since the open source release. (It even works via the same `--autocorrect`
mechanism.)

### Trivial instance variable declarations

Second, Sorbet relaxed the need for certain instance variable declarations. Like
constants, instance variables required type annotations at `typed: strict` or
else were treated as untyped. To understand which annotations aren't required
anymore, let's first recap how things used to work:

```ruby
# typed: strict
sig {params(x: Integer, y: String).void}
def initialize(x, y)
  @x = T.let(x, Integer) # ok
  @y = y # error: Use of undeclared variable `@y`
  puts @z # error: Use of undeclared variable `@z`
end
```

In this example, only `@x` has been declared with a type annotation (the
`T.let`). Both `@y` and `@z` are undeclared to Sorbet, and thus both reported
errors in `typed: strict`. Many people didn't like this. They wanted Sorbet to
treat `@y = y` as declaring the instance variable `@y`, and that's exactly how
it works now:

```ruby
# typed: strict
sig {params(x: Integer, y: String).void}
def initialize(x, y)
  @x = T.let(x, Integer) # still ok
  @y = y # ok (new!)
  puts @z # still an error
end
```

This applies when assigning an argument into an instance variable directly. As a
result, it's frequently enough to only add a signature when upgrading to
`typed: strict`: the instance variable type comes for free.

### Nilable instance variables

The third `typed: strict` feature we added also made it easier to work with
instance variables. Again, to understand what changed and why, let's take a look
at the previous state of things.

Sorbet previously required instance variables to be declared in `initialize`.
What if this weren't the case? Consider this example:

```ruby
class A
  def set_x_to_0
    # Declare @x as type Integer
    @x = T.let(0, Integer)
  end
  def x_plus_1
    # Is @x really an Integer here?
    @x + 1
  end
end
```

If Sorbet were to allow this code, its correctness would depend on whether
`set_x_to_0` was called first, before `x_plus_1`:

```ruby
# This is ok:
A.new.set_x_to_0.x_plus_1

# This explodes at runtime:
A.new.x_plus_1  # undefined method `+` for NilClass
```

By requiring instance variables to be declared in `initialize`, Sorbet ensures
they actually have the type they're annotated with. Sadly this ruled out using
`typed: strict` to type common Ruby idioms. For example, this module doesn't
have an `initialize` method (nor should it!) but still uses an instance
variable:

```ruby
# typed: strict
module B
  sig {returns(String)}
  def current_user
    # error: Use of undeclared variable `@current_user`
    @current_user ||= ENV.fetch('USER')
  end
end
```

The change we made is to allow instance variables to be declared **anywhere**,
as long as they're declared nilable. Sorbet still guarantees that **if** these
instance variables are initialized they have the right type, but doesn't make a
promise about whether or not they're initialized at all.

Using `typed: strict` with our `current_user` example from before now involves
just a single line to declare the type of the `@current_user` variable:

```ruby
# typed: strict
module B
  sig {returns(String)}
  def current_user
    # Declare @current_user as either String or nil:
    @current_user = T.let(@current_user, T.nilable(String))
    @current_user ||= ENV.fetch('USER')
  end
end
```

Collectively, these three features have made `typed: strict` adoption much
smoother, and we have the numbers to back it up. In Stripe's multi-million-line
Ruby codebase, over 85% of files are `typed: true` or above, and over 40% of all
files are `typed: strict` or above.

That's pretty much it for the `typed: strict` features, so with the next section
we're back to normal type system features.

> **Update, 2022-07-23**: The changes described in this section are now
> obsolete. We have changed Sorbet to allow declaring lazily-initialized
> instance variables in the natural way. Simply use
>
> ```ruby
> @current_user ||= T.let(ENV.fetch('USER'), T.nilable(String))
> ```
>
> like normal.

### Type checking database code with `T.attached_class`

Sorbet has had `T.class_of(MyClass)` [since it was open sourced][class-of-docs],
which allows passing class objects around as a values. But Sorbet lacked the
opposite feature: to refer to "an instance of the current singleton class." This
pattern is super common in real-world Ruby: it's how basically every ORM's API
is structured:

```ruby
# ActiveRecord:
Person.find(1) # => returns an *instance* of Person

# Stripe's internal ORM:
Charge.load_one('ch_some_id') # => returns an *instance* of Charge
```

It wasn't previously possible to write a return type for these methods, but it's
simple now, via a feature we call `T.attached_class`:

```ruby
class AbstractModel
  # Returns `T.attached_class`, or "an instance of
  # whatever the current singleton class is"
  sig {params(id: String).returns(T.attached_class)}
  def self.load_one(id)
    # ... calls self.new somewhere ...
  end
end
```

It's interesting to think about **why** typing these methods (`find`,
`load_one`, ...) wasn't possible before. At first glance, it almost seems like
these methods need a different type annotation depending on the call site. Call
`find` on the `Person` class to get back a `Person` instance, but call it on the
`Charge` class to get back a `Charge` instance. And while it looks like the type
needs to be different each time, these methods are only written once (inside the
framework itself) which means there can only be one sig that captures all these
different behaviors.

This problem—defining something once, but using it polymorphically at many
types—is a textbook use case for generics, and that's exactly how this feature
is implemented under the hood. Sorbet has had support for generics since we
open-sourced it, but they were mostly designed with the goal of typing the
standard library (like Array and Hash).

Usage of generics in application code is far more rare, and thus largely
untested by real-world code. A large part of implementing this feature involved
stabilizing the foundations on which generics in Sorbet are built, so that they
interact predictably with other features like subtyping and [control-flow
sensitivity][flow-sensitive-docs]. Important stabilizations that landed to
support `T.attached_class` include:

- Static variance checking for generic type members
- Upper and lower bounds on generic type members
- Exhaustiveness checks that work with generics the same ways as non-generics
- Many other subtle and important bug fixes

We expect that `T.attached_class` will be primarily useful for maintainers of
libraries and frameworks and is largely designed to be invisible to consumers of
those frameworks, so don't feel like you need to dive into this feature super
far. But if you're still curious, there's [more written in the
documentation][attached-class-docs].

## What's next: editor support

We could go on all day about what's happened the past six months, but instead
let's switch to what the Sorbet team plans to focus on next: Sorbet-powered IDE
features.

It's no secret that the Sorbet team has been working on editor tooling: we
mention it in nearly [all our talks][rubyconf2019], and people can already play
around with the editor features we've built so far in the [online Sorbet
playground][sorbet.run]. Hundreds of developers use features like Go to
Definition, Hover to see types and docs, and in-line type errors every week at
Stripe, all powered by Sorbet.

Sorbet is building on top of the [Language Server Protocol][lsp] (LSP), which
means the features it provides are editor-agnostic. Any editor with an LSP
client that supports all the features Sorbet implements will be able to benefit.
We're building LSP support [in the open][lsp.h] so it's technically possible to
try it out already, but the experience isn't up to our high standards just yet.
To get there, these claims should be true:

- It never raises an unhandled exception or otherwise crashes. This is almost
  true already, and we plan to make sure that we thoroughly test the
  implementation so that it stays that way. It's a terrible user experience to
  have a bug in Sorbet prevent you from using editor features.

* It works out of the box with at least one editor's LSP client implementation.
  We're currently targeting VS Code because we find its LSP support to be great
  and it's popular with a wide audience of developers. Out of the box support
  means we plan to open source a VS Code extension that people can install with
  a single click.

- It's blazingly fast. By most standards, Sorbet's editor support is already
  pretty fast (which you can see for yourself in [sorbet.run][sorbet.run]). But
  at Stripe, even small inefficiencies add up when multiplied across our
  multi-million-line Ruby codebase. As we put Sorbet's LSP implementation
  through the paces, we're making sure that it's as fast as it is correct.

There are a handful of other things left to do, which are mostly finishing
touches. And just like we mentioned in the community section above, we plan to
have at least one member of the Sorbet team answering editor-related questions
from the community, like we already do for Sorbet itself. All that said, we're
currently targeting early 2020 to release the VS Code extension publicly and
declare Sorbet's editor support ready for everyone.

From our experience, having a Sorbet-powered editor integration is a complete
game changer. The way people work and interact with Ruby code changes when they
get instant feedback from their editor about potential errors, about where
things are defined, and about what types various expressions have. Suffice it to
say, we're excited to share this with the rest of the community! Thanks for your
patience in the mean time.

## Wrap up

Thanks for reading! If you're interested in learning more:

- Check out the [Sorbet docs][docs]
- Come ask us questions [on Slack][slack]
- Watch our [most recent talk][rubyconf2019] at RubyConf 2019

Thanks again,\
— Jake "jez" Zimmerman, on behalf of the Sorbet team

[shopify-talk]: https://www.youtube.com/watch?v=v9oYeSZGkUw
[czi-talk]: https://chanzuckerberg.wistia.com/medias/mypzu8ie86
[slack]: https://sorbet.org/slack
[rbi-files]: https://sorbet.org/docs/rbi
[sorbet-typed]: https://github.com/sorbet/sorbet-typed
[union-types]: https://sorbet.org/docs/union-types
[exhaustiveness-docs]: https://sorbet.org/docs/exhaustiveness
[enums-docs]: https://sorbet.org/docs/tenum
[sealed-docs]: https://sorbet.org/docs/sealed
[typedness-docs]: https://sorbet.org/docs/static
[untyped-docs]: https://sorbet.org/docs/untyped
[suggest-sig-example]:
  https://sorbet.run/#%23%20typed%3A%20strict%0Adef%20hello%0A%20%20'Hello%2C%20world!'%0Aend
[class-of-docs]: https://sorbet.org/docs/class-of
[flow-sensitive-docs]: https://sorbet.org/docs/flow-sensitive
[attached-class-docs]: https://sorbet.org/docs/attached-class
[rubyconf2019]: https://www.youtube.com/watch?v=jielBIZ40mw
[lsp]: https://microsoft.github.io/language-server-protocol/
[lsp.h]: https://github.com/sorbet/sorbet/blob/master/main/lsp/lsp.h
[docs]: https://sorbet.org
