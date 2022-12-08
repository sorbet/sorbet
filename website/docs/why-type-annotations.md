---
id: why-type-annotations
title: Why does Sorbet sometimes need type annotations?
sidebar_label: Why type annotations?
---

## ... for methods?

Sorbet never attempts to infer types for methods. This is a key part of what
allows Sorbet to type check a codebase quickly. It means that the result of
doing inference on one method can never affect the result of doing inference on
another method, meaning that all methods can be typechecked entirely in
parallel.

In [`# typed: strict`](static.md) files, Sorbet requires type annotations for
methods, so that programmers have an explicit reminder that Sorbet does not do
method signature type inference.

## ... for constants?

Similarly, Sorbet only attempts to do type inference for constants when the type
of the constant is knowable without needing to do type inference.

This means that simple constants like `X = ""` or `Y = 1` do not need type
annotations—Sorbet can syntactically see that the type of these constants are
`String` and `Integer` respectively.

However, to know the type of constant assignments like `A = MyClass.new` or
`B = 1 + 1`, Sorbet needs to know the result type of the `new` and `+` methods,
respectively. To know a method's result type, Sorbet has to do type inference,
and Sorbet does not do type inference until all constants have been given type
annotations (which would be a cycle). Keep in mind that Sorbet respects
overloaded and redefined methods, so even simple expressions like these do not
always have well-known result types.

(**Note**: Newer versions of Sorbet will attempt to assume that the type of
`A = MyClass.new` is in fact `MyClass`, and require an explicit annotation
_only_ when that assumption turns out to be incorrect, for example due to an
override.)

## ... for instance variables?

Sorbet always requires type annotations for instance and class variables, with a
similar justification as the previous point for constants.

There is one exception, which is when an instance variable is declared in the
body of an `initialize` method by being assigned a variable whose type was
mentioned in the signature for `initialize`:

```ruby
class A
  sig {params(x: Integer).void}
  def initialize(x)
    @x = x
  end
end
```

In cases like these, Sorbet infers that the type of `@x` should be `Integer`,
without needing an explicit annotation. This means that if `@x` should be
allowed to store more types than just `Integer` (for example, maybe it should
also be allowed to store `nil`), an explicit annotation becomes required.

## ... for local variables?

Sorbet requires type annotations when widening the type of a variable within a
loop. This, again, is for performance. Sorbet's inference algorithm is very
simplistic—it examines each expression in a method body at most once. (Other
inference algorithms use separate type constraint generation and type constraint
solving passes over a method body to infer types, leading to fewer required type
annotations but potentially slower performance.)

Because real-world code has branches and loops, and Sorbet must pick a single
order to examine each individual expression in, Sorbet sometimes type checks
statements before knowing any updates that might happen to that variable.
Concretely:

```ruby
x = 123
2.times do
  x + 1
  x = nil
end
```

By the time Sorbet typechecks the `x + 1` line, Sorbet thinks that `x` has type
`Integer`, and says that expression has no error. Then on the next line, the
type of `x` is changed to `nil`, which would then introduce a runtime exception
when the `x + 1` is encountered on the second iteration of the loop. Since
Sorbet decides whether `x + 1` typechecks before looking at the `x = nil`
assignment, it has to report an error on `x = nil`.

### Couldn't Sorbet at least special case `true` and `false`?

Sorbet's type inference system is smart enough to track things like this:

```ruby
sig {params(banking_account: String).returns(T::Boolean)}
def is_risky_merchant(banking_account); ...; end

sig {params(banking_account: T.nilable(String)).void}
def example(banking_account)
  should_check_balance = false

  if banking_account && is_risky_merchant(banking_account)
    should_check_balance = true
  end

  if should_check_balance
    T.reveal_type(banking_account) # error: `String`
  end
end
```

[→ View full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Breturns%28T%3A%3ABoolean%29%7D%0Adef%20boolean_true%3B%20true%3B%20end%0Asig%20%7Breturns%28T%3A%3ABoolean%29%7D%0Adef%20boolean_false%3B%20false%3B%20end%0A%0Asig%20%7Bparams%28banking_account%3A%20String%29.returns%28T%3A%3ABoolean%29%7D%0Adef%20is_risky_merchant%28banking_account%29%3B%20true%3B%20end%0A%0Asig%20%7Bparams%28banking_account%3A%20T.nilable%28String%29%29.void%7D%0Adef%20desired_behavior%28banking_account%29%0A%20%20should_check_balance%20%3D%20false%0A%0A%20%20if%20banking_account%20%26%26%20is_risky_merchant%28banking_account%29%0A%20%20%20%20should_check_balance%20%3D%20true%0A%20%20end%0A%0A%20%20if%20should_check_balance%0A%20%20%20%20T.reveal_type%28banking_account%29%20%23%20error%3A%20%60String%60%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28banking_account%3A%20T.nilable%28String%29%29.void%7D%0Adef%20incorrect_behavior%28banking_account%29%0A%20%20should_check_balance%20%3D%20boolean_false%0A%0A%20%20if%20banking_account%20%26%26%20is_risky_merchant%28banking_account%29%0A%20%20%20%20should_check_balance%20%3D%20boolean_true%0A%20%20end%0A%0A%20%20if%20should_check_balance%0A%20%20%20%20T.reveal_type%28banking_account%29%20%23%20error%3A%20%60String%60%0A%20%20end%0Aend)

In this example, Sorbet knows that `banking_account` on the indicated line
actually has type `String`, not type `T.nilable(String)`. It knows this despite
the `if` guard checking whether the `should_check_balance` variable is `true`,
not whether the `banking_account` is non-`nil`. To achieve this, Sorbet
maintains sophisticated sets of implications saying "if we're in an environment
where a certain variable is truthy, then another variable must have a certain
type."

If Sorbet blindly assumed that `false` and `true` literals had type
`T::Boolean`, it would forget in which individual branches the variable had type
`TrueClass` or `FalseClass`, and be unable to maintain these knowledge sets.
Real-world code depends on patterns like this surprisingly frequently.

We have decided that the error message for changing a variable in a loop is very
clear, has an autocorrect, and the resulting `T.let`'d code is very obvious. But
if we did it the other way, sometimes requiring people to explicitly annotate
`T.let(true, TrueClass)`:

1. this pattern would look odd ("of course `true` has type `TrueClass`, isn't
   this annotation useless?"), and also
2. we wouldn't be able to easily build good error messages to suggest people to
   do this in the first place. Instead, the error messages would be reported far
   downstream from where the error actually happened, and be reported as
   something confusing like "this thing might sometimes be `nil`."

## Type annotations and strictness levels

Sorbet allows the programmer to opt-in to greater levels of static type rigor.
At lower [strictness levels](static.md), Sorbet allows definitions to be
implicitly untyped and therefor doesn't require type annotations.

At the `# typed: strict` level, Sorbet starts requiring explicit type
annotations on any definitions where it would have otherwise assumed a type of
`T.untyped`. (This is similar to TypeScript's `noImplicitAny` flag, for those
familiar with it.)

Specifically, in a `# typed: strict` file it's an error to omit type annotations
for:

- methods
- instance variables
- class variables
- constants

It may seem counterintuitive that Sorbet does _not_ require type annotations in
a file marked `# typed: true`, but this is an intentional part of Sorbet's
implementation of [gradual typing](gradual.md). In the `# typed: true`
strictness level, unannotated methods, instance variables, and constants are
assumed to be `T.untyped`. This allows a programmer to write untyped or
partially-typed definitions while still benefiting from type checking when
static type information is present.
