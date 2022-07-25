---
id: exhaustiveness
title: Exhaustiveness (T.absurd)
---

**Exhaustiveness checking** is a feature of a language where the type checker
guarantees that the programmer has covered all cases. It can be super useful at
catching pesky edge cases before they become bugs, and Sorbet supports it as a
first class feature.

<!-- Using [Flow-Sensitive Typing](flow-sensitive.md), [Union Types](union-types.md), and [Type Assertions](type-assertions.md) we can approximate exhaustiveness checking. -->

## tl;dr

If you already know what exhaustiveness checking is, you might just want to see
the end result:

```ruby
# typed: true
extend T::Sig

class A; end
class B; end
class C; end

# (1) Define a type alias as a union type of A, B, or C
AorBorC = T.type_alias {T.any(A, B, C)}

sig {params(x: AorBorC).void}
def foo(x)
  # (2) Use flow-sensitivity to cover each case separately
  case x
  when A
    # To re-iterate: within this branch, Sorbet knows x is an A
    T.reveal_type(x) # Revealed type: `A`
  when B
    T.reveal_type(x) # Revealed type: `B`
  else
    # (3) Use T.absurd to ask Sorbet to error when there are missing cases.
    T.absurd(x) # error: didn't handle case for `C`
  end
end
```

And some quick notes:

1.  Exhaustiveness checks are **opt-in**, not enabled by default. This is
    primarily to make it easier to adopt Sorbet in existing projects.

1.  `T.absurd(...)` is implemented both statically and at runtime. Statically
    Sorbet will report an error, and at runtime Sorbet will raise an exception.

1.  Sorbet will error statically if the condition to a case statement using
    `T.absurd` is `T.untyped`. This prevents against losing exhaustiveness
    checking due to a change in the code that weakens static type information.

Now let's walk through an example explaining not only **how** Sorbet provides
exhaustiveness checking, but also **why** it's useful:

## Example

Let's say we have this setup:

```ruby
# typed: true
extend T::Sig

class A; end
class B; end

sig {params(x: T.any(A, B)).void}
def foo(x)
  # ...
end
```

There are two classes (`A` and `B`), and our method `foo` takes either `A` or
`B`.

In the body of `foo`, we'd like to do something different when given an `A`
versus when given a `B`:

```ruby
# ...

def do_a(a); puts 'Got an A!'; end
def do_b(b); puts 'Got a B!'; end

sig {params(x: T.any(A, B)).void}
def foo(x)
  # Problematic! We'll improve this shortly...
  case x
  when A
    do_a(x)
  when B
    do_b(x)
  end
end
```

There's no bug here yet, but consider that some time in the future, someone
wants to update `foo` to work with class `C`:

```ruby
# ...

class C; end
def do_c(c); puts 'Got a C!'; end

# ...

sig {params(x: T.any(A, B, C)).void}
def foo(x)
  # Bug! We forgot to update the body to handle C...
  case x
  when A
    do_a(x)
  when B
    do_b(x)
  end
end
```

In this case, there's a silent bug in our program. We've updated the signature
of `foo` to accept instances of `C`, but we haven't updated the method body to
actually do something with it!

**Exhaustiveness checking** is a feature that turns this kind of bug into a type
error. It lets us catch the problem statically before causing all sorts of
problems down the line.

We can enable exhaustiveness checking in Sorbet using `T.absurd(...)`:

```ruby
sig {params(x: T.any(A, B, C)).void}
def foo(x)
  case x
  when A
    do_a(x)
  when B
    do_b(x)
  else
    # We're not handling all the cases, so Sorbet will report an error:
    T.absurd(x) # error: didn't handle case for `C`
  end
end
```

In this case, Sorbet is telling us that by the time we got to the else case, we
were missing a case: both `B` and `C` needed to be handled, but we were only
handling `B`. `T.absurd` should be the same variable that the `case` statement
discriminates on.

And as one last tip, we can use [Type Aliases](type-aliases.md) to give a name
to `T.any(A, B, C)` and reuse it throughout our codebase. This means we can
update the alias in one place, instead of at every individual method!

```ruby
AorBorC = T.type_alias {T.any(A, B, C)}

sig {params(x: AorBorC).void}
def foo(x)
  # ...
end
```

## Using `T.absurd` to assert a dead condition

Note that exhaustiveness checking via `T.absurd` is merely a mode of use of
checking that a particular conditional branch is unreachable. Sorbet allows
using `T.absurd` to assert that arbitrary conditions are unreachable:

```ruby
sig {params(x: Integer).void}
def example(x)
  if x.nil?
    # `x` must never be `nil`
    T.absurd(x)
  end

  # ...
end
```

This can be used to assert that, for example, the `example` method is never
refactored in such a way that `x` is allowed to become possibly `nil`. If such a
refactor happened in the future, Sorbet would flag that the `T.absurd` call was
in fact reachable.

Recall that since `T.absurd` will also complain that the condition is reachable
if `x` ever becomes `T.untyped`, so `T.absurd` can be used to assert not only
that a variable has a given type, but also that the type is known statically.

## What's next?

- [Flow-Sensitive Typing](flow-sensitive.md)

  Sorbet implements a control flow-sensitive type system, which means it tracks
  the flow of control through a program and narrows or widens the types of
  variables in response. Flow-sensitive typing is the feature that ultimately
  powers exhaustiveness.

- [Sealed Classes and Modules](sealed.md)

  The form of exhaustiveness checking seen here relied on simultaneously
  updating a type alias when adding a new case to consider. An alternative to
  this is to use sealed classes, which effectively make exhaustiveness a
  property of the definition not the usage site.

- [Abstract Classes and Interfaces](abstract.md)

  The form of exhaustiveness we've seen here is structural—Sorbet checks that
  each case of a particular structure have been handled. An alternative is to
  describe an abstract method (i.e., behavior) and require that all subclasses
  implement that method (a form of "behavioral exhaustiveness"). This doc
  describes how to use Sorbet's abstract classes and methods to enforce those
  guarantees.
