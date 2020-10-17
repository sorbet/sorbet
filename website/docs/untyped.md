---
id: untyped
title: T.untyped
---

The type `T.untyped` represents a type that Sorbet has no specific knowledge
about. Instance variables and constants that have not been given a static type
with `T.let` or another [type assertion](type-assertions.md) are assumed to have
the type `T.untyped`, and methods that have not been given a signature with
`sig` are assumed to take arguments of type `T.untyped` and return a value of
type `T.untyped`.

Sorbet will not check any operations performed on values of type `T.untyped`: it
will accept any method calls on them with any number of arguments of any types:

```ruby
# typed: true
class A; end
A.new.foo   # Method foo does not exist on A
T.let(A.new, T.untyped).foo  # No errors
```

## What is untyped?

In the absence of a `sig`, Sorbet will assume that the method takes values of
`T.untyped` for all its parameters, and returns something of type `T.untyped`.
Consequently, we first start using Sorbet with a codebase, it will be filled
with `T.untyped`. When Sorbet examines a method without a `sig`, it can only
deduce that the arguments to the method are of type `T.untyped` and that
whatever is returned from the method is of type `T.untyped`.

It's also possible to use `T.untyped` as a type explicitly. For example, the
following is a perfectly valid typed method definition:

```ruby
extend T::Sig
# typed: true
sig { params(x: T.untyped, y: T.untyped).returns(T.untyped) }
def plus(x, y); x + y; end
```

In this case, the `sig` on `plus` is explicit version of the implicit signature
that Sorbet would assume of `plus` in the absence of any other information. It
would be easy for a programmer to deduce a reasonable static type for `plus`
just by looking at the body of this simple method, but in a bigger codebase with
more complicated methods, it can be useful to start with a type signature that
includes parameters of type `T.untyped` and then replace those with more
specific types as we go.

## Converting from untyped

In general, there's nothing special that needs to be done to use an untyped
value in a typed context. If a method accepts an `Integer`, then it will also
accept a `T.untyped` value in the same place:

```ruby
# typed: true
extend T::Sig
sig { params(x: Integer).returns(Integer) }
def incr(x); x + 1; end

n = T.let(5, T.untyped) # this value is untyped...
puts incr(n) # but Sorbet accepts it when passed to a
             # function that expects an Integer
```

In this case, it's obvious to a reader of the code that `n` has the expected
type `Integer` at runtime, but what if it didn't? In that case, Sorbet will
still accept the program during typechecking, but the program will produce an
error at runtime due to Sorbet's [runtime checks](runtime.md). Consequently, the
method body of a method like `incr` can be _guaranteed_ that it was passed
parameters of the correct types, either because of compile-time validation or
because we double-checked at runtime.

It's also possible to be more proactive about checking runtime types. This can
be done with features like Ruby's `is_a?` method or `case` statement, which can
be used to check whether something is an instance of a given class. Because
Sorbet supports [flow-sensitive typing](flow-sensitive.md), it can use these
checks to refine type information in contexts where such tests have succeeded.
In the example below, `a` starts out as `T.untyped`, but if `a.is_a? Integer` is
true, then Sorbet can surmise that within the body of that conditional, `a` must
be an `Integer`.

```ruby
# typed: true
a = T.let(5, T.untyped)
T.reveal_type(a)  # a has type T.untyped

if a.is_a? Integer
  T.reveal_type(a)  # a has type Integer
end
```

## Converting to untyped

There might be some situations in which we might want to "forget" static type
information and treat a value as though it were untyped. The
[escape hatch](troubleshooting.md#escape-hatches) Sorbet provides for this is
called `T.unsafe`, which takes anything and returns that same thing but as
`T.untyped`.

There's a reason this is named `T.unsafe`, and it is because converting things
to `T.untyped` **weakens static type guarantees**, and should be done with
caution. The value of a tool like Sorbet is that it can analyze code and produce
guarantees about that code's behavior before that code ever runs, so weakening
the static type information available to Sorbet will also weaken the guarantees
that Sorbet provides. On the other hand, especially with codebases that have not
been fully converted over to static types, it can be useful to have such an
escape hatch. More specifics about how to use how to use `T.unsafe` and when it
might be useful are explained in the
[troubleshooting section on `T.unsafe`](troubleshooting.md#tunsafe).
