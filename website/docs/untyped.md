---
id: untyped
title: T.untyped
---

> TODO: This page is still a fragment. Contributions welcome!

The type `T.untyped` represents a type that Sorbet has no specific knowledge
about. Instance variables and constants that have not been given a static type
with `T.let` or another [type assertion](type-assertions.md) are assumed to have
the type `T.untyped`, and methods that have not been given a signature with
`sig` are assumed to take arguments of type `T.untyped` and return a value of
type `T.untyped`.

When you have a value of type `T.untyped`, Sorbet will not check any operation
you do with it: you can call any method with as many or few arguments as you
want, with whatever types you want:

```ruby
# typed: true
class A; end
A.new.foo   # Method foo does not exist on A
T.let(A.new, T.untyped).foo  # No errors
```

## What is untyped?

In the absence of a `sig`, Sorbet will assume that the method takes any number
of `T.untyped` parameters, and returns something of type `T.untyped`.
Consequently, when you first start using Sorbet, your whole codebase will be
filled with `T.untyped`. If you have a method without a `sig`, then within the
body of the method, its arguments will be implicitly understood to have the
static type `T.untyped`, and any values returned from such a method would also
have the static type `T.untyped`.

You can also explicitly use `T.untyped` as a type anywhere that you would use a
type otherwise. For example, the following is a perfectly valid typed method
definition:

```ruby
extend T::Sig
# typed: true
sig { params(x: T.untyped, y: T.untyped).returns(T.untyped) }
def plus(x, y); x + y; end
```

In this case, the `sig` we wrote is an explicit version of the implicit
signature that Sorbet would assume of `plus` in the absence of any other
information. Here we can probably deduce a reasonable static type for `plus`
just by looking at the body of the method, but in a bigger codebase, it can be
useful to start with a type signature that includes parameters of type
`T.untyped` and then replace those with more specific types as we go.

## Converting from untyped

In general, there's nothing special you need do if you want to use an untyped
value in a typed context: if you have a method that accepts an `Integer`, and
you have an untyped value that you believe is an `Integer`, then you can simply
pass it as an argument and the Sorbet typechecker will accept it:

```ruby
# typed: true
extend T::Sig
sig { params(x: Integer).returns(Integer) }
def incr(x); x + 1; end

n = T.let(5, T.untyped) # this value is untyped...
puts incr(n) # but Sorbet accepts it when passed to a
             # function that expects an Integer
```

In this case, we as programmers know that `n` has the right runtime type, but
what if we're wrong? In that case, Sorbet will still accept the program during
typechecking, but the program will produce an error at runtime due to Sorbet's
[runtime checks](runtime.md). Consequently, the method body of a method like
`incr` can be _guaranteed_ that it was passed parameters of the correct types,
either because of compile-time validation or because we double-checked at
runtime.

What if we want to be a little bit more proactive about checking runtime types?
In that case, we can use features like Ruby's `is_a?` method or `case` statement
which allow us to check whether something is an instance of a given class.
Because Sorbet supports [flow-sensitive typing](flow-sensitive.md), it can use
these checks to refine type information in contexts where the test applies. In
the above example, `a` starts out as `T.untyped`, but if `a.is_a? Integer` is
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

There might be some situations in which you might want to "forget" static type
information and treat a value as though it were untyped. The
[escape hatch](troubleshooting.md#escape-hatches) Sorbet provides for this is
called `T.unsafe`, which takes anything and returns returns that same things but
as `T.untyped`.

There's a reason this is named `T.unsafe`, and it is because converting things
to `T.untyped` **weakens your static type guarantees**, and should be done with
caution. The value of a tool like Sorbet is that it can analyze your code before
it ever runs to guarantee things, and getting rid of static type information
will also get rid of various guarantees. That said, especially with codebases
that have not been fully converted over to static types, it can be useful to
have such an escape hatch. More specifics about how to use how to use `T.unsafe`
and when you might want to use it are explained in the
[troubleshooting section on `T.unsafe`](troubleshooting.md#tunsafe).
