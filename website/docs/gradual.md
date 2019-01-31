---
id: gradual
title: Gradual Type Checking & Sorbet
sidebar_label: Gradual Type Checking
---

Sorbet is a gradual type checker, which is a blessing and a curse.

## What is a gradual type system?

In large codebases, complete rewrites are expensive and dangerous. A complete
halt on new features to pay down technical debt? Oof. "Ah, but if we just
rewrite in _\<my favorite language\>_, we'll be fine!" Maybe... this risk pays off
only in extreme circumstances.

**Gradual type systems** solve this. Types can be adopted incrementally—team
by team or file by file—according to the velocity of individual teams. In the
face of large user asks, quarterly planning, and re-orgs, migrations to gradual
type systems are resilient to shifting priorities.

To deliver on this promise, gradual type systems make a compromise: type checks
can be "turned off" at any time and at any level of granularity. In Sorbet, we
can opt out of type checking at a call site, a method definition, a class, or
even an entire file.

This is the fundamental tension in a gradual type system: when a program
typechecks, it's not clear how confident we can be in the typechecker's
assessment of our code. But it's precisely this feature that lets us plow
forward adding types while simultaneously adding features.


## T.untyped: A double-edged sword

The defining characteristic of a gradual type system is to be able to turn it
off at any granularity. So what _specifically_ makes Sorbet gradual? The answer:
[`T.untyped`](untyped.md).

`T.untyped` is a **type** in Sorbet's type system, but it's unlike any type
found in say, Go or Java. `T.untyped` has two special properties:

1.  Every value can be asserted to have type `T.untyped`.
2.  Every value of type `T.untyped` can be asserted to be _any other type_!

If this sounds counter intuitive, that's because it is. Let's see why with an
example:

```ruby
# T.let asserts that an expression has a type:
x = T.let(0, Integer)

# Anything can be T.untyped:
y = T.let(x, T.untyped)

# and T.untyped can be anything!
z = T.let(y, String)

T.reveal_type(x) # => Integer
T.reveal_type(y) # => T.untyped
T.reveal_type(z) # => String

# ... z = 0, but its type is String!
```

Follow what this example is doing: we start with `x = 0`, then set `y = x`,
then `z = y`, so in the end `z = 0`. But at the same time, Sorbet thinks that
`z` has type String!

At first blush, it looks like we **do not** want this property in our type
system. It lets us make wildly inaccurate claims! But let's withhold judgement
for a second, and see what this property enables:

```ruby
# --- File we haven't typed yet ---

class Person
  attr_accessor :name
end

# --- File we're currently typing ---

sig {params(person: Person).returns(Integer)}
def name_length(person)
  name = person.name # (*)
  T.reveal_type(name) # => T.untyped

  len = name.length
  T.reveal_type(len) # => T.untyped

  len
end
```

Look at the line marked `(*)`. Sorbet knows about the `Person` class, and knows
that it has a method `#name`, but implicitly assumes this method's return type
is `T.untyped`. Why? This is what Sorbet assumes for any methods that don't have
a corresponding `sig` annotation.

Since `T.untyped` _could be_ any type, when `name.length` runs Sorbet
optimistically assumes that `name` _will_ have some type which has a `#length`
method. Then it also optimistically assumes that the thing `#length` returns
(in this case, `len`) could be an `Integer`. Thus, Sorbet declares that this
method type checks.

In a sense, "being optimistic" is really just a way of the programmer telling
Sorbet, "I believe this code is correct." And realistically, we might be
convinced of our code's correctness for a number of reasons:

- because this code is well tested,
- because this code has been running in production just fine, or
- because we've carefully reviewed the code for correctness.

But as many of us have experienced, our faith is frequently misplaced when it
comes to believing that some code is correct. As we add type annotations, Sorbet
helps uncover and check hidden assumptions, providing even more evidence in
favor of our code's correctness. The next question becomes: what if our
annotations are wrong?


## Checking our assertions at runtime

In our `name_length` example above, we added a type annotation to a previously
untyped method. What if our annotation was wrong? For example, what if we
accidentally typed the return as a String:

```ruby
class Person
  attr_accessor :name
end

# The annotation for .returns is wrong!
sig {params(person: Person).returns(String)}
def name_length(person)
  person.name.length
end

person = Person.new
person.name = 'Jenny Rosen'
name_length(person)
```

Because `Person#name` returns `T.untyped`, Sorbet does not have enough
information statically to check that `#length` returns an Integer. But if we
were to run this code, we'd know at the moment our method returns that our
result didn't match our declaration.

Enter the [runtime system](runtime.md). At runtime, the `sig` above a method def
wraps our method into a new method which does three things:

- checks that it was called with arguments matching the `sig`'s declared params
- calls our method and gets the result
- checks that our result matches the `sig`'s declared return

This means we can leverage our existing test coverage and our production
assertion monitoring to build confidence that our type annotations are correct.
In the early days of [adopting Sorbet](adopting.md) in a codebase this is super
valuable because **most** things are going to be `T.untyped`!

## The lifecycle of T.untyped

By now we have a pretty good understanding of `T.untyped`:

- It's useful for spurring short term adoption.
- It harms our long term ability to statically analyze our code.

When a codebase is adopting a gradual type checker we see an adoption curve
where `T.untyped` shows up everywhere then slowly gets phased out. There are
three main phases of `T.untyped` in a codebase:

1.  The initial ramp up, where only early adopters are using types.
1.  The transitional period, where a codebase's core abstractions gain types.
1.  The long tail, where typed code becomes the majority.

For context, at Stripe we're somewhere about halfway between (2) and (3). We've
already enabled `typed: true` in the vast majority of files in our main
codebase, but most methods and even a few core abstractions are not typed yet.

There are a number of tools for managing `T.untyped` in a codebase:

- [Strictness levels](static.md) (`typed: true`, `typed: strict`, and `typed:
  strong`).
  - These levels control when `T.untyped` is allowed.
- [`sig`](sigs.md) and [`T.let` annotations](strict.md).
  - Adding annotations opts code into static type checking.
- [`T.unsafe` and `T.cast`](troubleshooting.md#escape-hatches).
  - These helpers opt code out of static checks.
- [`T.reveal_type`](troubleshooting.md) and our editors.
  - Knowing which things are `T.untyped` is half the battle.

These tools are the bread and butter for rolling out types in a gradual type
system. Mostly that means they're tools that the Developer Productivity team
uses, but understanding that they're there can help give context when
troubleshooting. See [Troubleshooting](troubleshooting.md) for more information.


## What's next?

Gradual type systems make it tractable to mix feature development with adopting
a typed language. The guarantees they provide are necessarily weaker than
languages without a type like `T.untyped`, because `T.untyped` allows us to
"opt out" of type checking. But it's precisely this property which enables
incremental adoption through a large codebase.

Armed with this knowledge, here are some resources to check out next:

- [Enabling Static Checks](static.md)

  How to opt into higher strictness levels and add type annotations so that
  Sorbet can report more errors.

- [Enabling Runtime Checks](runtime.md)

  Learn more about what benefits the runtime system provides, and how to work
  with it.
