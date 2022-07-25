---
id: noreturn
title: T.noreturn
---

The type `T.noreturn` is an "impossible" type: zero values in the language have
this type. This type has a number of key use cases.

## Declaring that a method always raises

`T.noreturn` can declare that a method never returns (for example, because it
unconditionally raises).

```ruby
sig {returns(T.noreturn)}
def raise_always
  raise RuntimeError
end

sig {returns(T.noreturn)}
def exit_program
  exit
end
```

In fact, neither `raise` nor `exit` are keywords in Ruby but are in fact methods
(defined on the `Kernel` module in the standard library). Sorbet uses an
[RBI file](rbi.md) to declare that
[`raise` and `exit` never returns](https://github.com/sorbet/sorbet/blob/a11ae1b427def972a6b6eb203c0d676f0f77ddae/rbi/core/kernel.rbi#L3104-L3127),
among others.

## Detecting unreachable code

`T.noreturn` powers Sorbet's unreachable (dead) code analysis, because it is
impossible to have a value of type `T.noreturn`. For example:

```ruby
sig {params(x: T.nilable(Integer)).void}
def example(x)
  if x.nil?
    raise ""
    puts(x) # error: This code is unreachable
  end
end
```

The call to `puts(x)` happens after the call to `raise`. Since Sorbet knows that
`raise ""` evaluates to `T.noreturn`, it knows that the call to `puts(x)` will
never run. (The error message makes no mention of `T.noreturn` explicitly, but
this is in fact what powers the underlying analysis.)

Note that it can be useful to assert that a given branch of control flow
**must** be unreachable. For these cases, Sorbet provides
[`T.absurd`](exhaustiveness.md#using-tabsurd-to-assert-a-dead-condition), which
also handles exhaustiveness checking (a special case of asserting that a branch
of code is unreachable).

## Writing infinite loops

Another way a method can never return is if it infinitely loops, either by using
`loop {...}` or by using a `while` loop with an unconditionally `true`
condition, neither of which ever `break` out of the loop:

```ruby
sig {returns(T.noreturn)}
def loop_forever
  loop do
    # ...
  end
end

sig {returns(T.noreturn)}
def while_forever
  while true
    # ...
  end
end
```

Infinite loops like this occur frequently in long-lived processes that are
designed to respond to user input indefinitely.

Having type system support for this may not seem immediately useful at first,
but ties into the previous section: Sorbet can detect code written after an
infinite loop, and flag that the code is unreachable.

## Detecting impossible-to-call methods

Because of [intersection types](intersection-types.md), it is possible to create
types for which there cannot be any values. If there are no values of a given
type, and that type is used for an argument of a method, Sorbet flags the usage
as dead code:

```ruby
class A; end
class B; end

sig {params(x: T.all(A, B)).void}
def example(x)
  puts(x) # error: This code is unreachable
end
```

As discussed in
[Understanding how intersection types collapse](intersection-types.md#understanding-how-intersection-types-collapse),
`T.all(A, B)` collapses to `T.noreturn` because the classes `A` and `B` are
completely unrelated, and it therefore impossible for a value to simultaneously
be an instance of `A` and `B`.

In these examples, Sorbet has caught an example of a method which is not
possible to call.

## `T.noreturn` and subtyping

`T.noreturn` is a subtype of all other types. This can be useful in certain
situations. For example, the type of the empty array is most accurately typed as
`T::Array[T.noreturn]`. Because `T.noreturn` is a subtype of all other types,
this means that `[]` has type `T::Array[Integer]` and `T::Array[String]` and
`T::Array[T.any(Integer, String)]`, etc.

This also allows Sorbet to detect when, for example, there's an incorrect
attempt to access the first element of a known-empty list:

```ruby
xs = T::Array[T.noreturn].new
x = xs.fetch(0)
puts(x) # error: This code is unreachable
```

## `T.noreturn` and `T.untyped`

Despite what we've claimed throughout this page, technically it **is** possible
to have a value of type `T.noreturn` by way of `T.untyped`:

```ruby
sig {returns(T.noreturn)}
def always_raises
  T.unsafe("Just kidding, it actually does return")
end
```

This is an unavoidable downside of Sorbet's use of [`T.untyped`](untyped.md) to
implement a [gradual type system](gradual.md).

This is a major reason why Sorbet's [runtime type checking](runtime.md) is
valuable: in order for Sorbet's static dead code analysis to be accurate, Sorbet
relies on the fact that static violations of the declared types will be caught
at runtime by checking [method signatures](sigs.md) and
[type assertions](type-assertions.md).

Disabling runtime checks will not change anything about what Sorbet infers
statically--in particular, Sorbet will still believe it is impossible to have a
value of type `T.noreturn` and will still report the same unreachable code
errors it normally would.

[daemon processes]: https://en.wikipedia.org/wiki/Daemon_(computing)
