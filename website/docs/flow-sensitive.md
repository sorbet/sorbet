---
id: flow-sensitive
title: Flow-Sensitive Typing
sidebar_label: Flow-Sensitivity (is_a?, nil?)
---

Sorbet implements a **control flow-sensitive** type system. It models control
flow through a program and uses it to track the program's types more
accurately.[^control]

<!-- prettier-ignore-start -->

[^control]: We abbreviate "control flow-sensitive" to "flow-sensitive"
throughout these docs, because Sorbet does little to no _data flow analysis_.
(Data flow analysis is a separate family of techniques that models the way data
flows between variables in a program.)

<!-- prettier-ignore-end -->

## Example

```ruby
extend T::Sig

sig {params(x: T.nilable(String), default: String).returns(String)}
def maybe(x, default)
  # (1) Outside the if, x is either nil or a String
  T.reveal_type(x) # => Revealed type: `T.nilable(String)`

  if x
    # (2) At this point, Sorbet knows `x` is not nil
    T.reveal_type(x) # => Revealed type: `String`

    x
  else
    # (3) In the else branch, Sorbet knows `x` must be nil
    T.reveal_type(x) # => Revealed type: `NilClass`

    default
  end
end
```

In this example, we ask Sorbet ([using T.reveal_type](troubleshooting.md)) what
the type of `x` is at three places, and get different answers each time:

- Outside the `if`, Sorbet only knows what the `sig` said about `x`.
- In the `if` branch, Sorbet knows `x` is **not** `nil`.
- In the `else` branch, Sorbet knows `x` **must** be `nil`.

## Predicates

Sorbet bakes in knowledge of a bunch of Ruby constructs out of the box:

```ruby
# typed: true
extend T::Sig

sig {params(x: Object).void}
def flow_sensitivity(x)
  # (1) is_a?
  if x.is_a?(Integer)
    T.reveal_type(x) # => Integer
  end

  # (2) case expressions with Class#===
  case x
  when Symbol
    T.reveal_type(x) # => Symbol
  when String
    T.reveal_type(x) # => String
  end

  # (3) comparison on Class objects (<)
  if x.is_a?(Class) && x < Integer
    T.reveal_type(x) # => T.class_of(Integer)
  end
end
```

The complete list of constructs that affect Sorbet's flow-sensitive typing:

- `if` expressions / `case` expressions
- `is_a?` / `kind_of?` (check if an object is an instance of a specific class)
- `nil?`
- `blank?` / `present?` (these assume a Rails-compatible monkey patch on both
  `NilClass` and `Object`)
- `Module#===` (this is how `case` on a class object works)
- `Module#<`, `Module#<=` (like `is_a?`, but for class objects instead of
  instances of classes)
- Negated conditions (including both `!` and `unless`)
- Truthiness (everything but `nil` and `false` is truthy in Ruby)
- `block_given?` (internally, this is a special case of truthiness)

> **Warning**: Sorbet's analysis for these constructs hinges on them not being
> overridden! For example, Sorbet can behave unpredictably when overriding
> `is_a?` in weird ways.

## What about `respond_to?`?

Sorbet cannot support flow sensitivity for `respond_to?` in the way that most
people expect.

For example:

```ruby
sig {params(x: Object).void}
def flow_sensitivity(x)
  # Does not work:
  if x.respond_to?(:foo)
    T.reveal_type(x) # => Object
    x.foo # Method `foo` does not exist
  end
end
```

In this example, knowing that `x` responds to a method with the name `foo` does
not tell Sorbet anything more specific about the type for `x`. Sorbet does not
have any sort of duck-typed interfaces that let Sorbet update its knowledge of
the type of `x` to "`Object` plus responds to `foo`", so it must keep the type
of `x` at `Object`, which does not allow calling `foo`.

Note that even if Sorbet did support such a type, it's likely that
flow-sensitive type updates for `respond_to?` would _still_ not be supported,
because knowing that the method `foo` exists says nothing about what parameters
that method expects, what their types are, or what the return type of that
function is.

It's possible that someday that Sorbet could support a limited form of
`x.respond_to?(:foo)` when one of the component types of `x` is a type which has
a known method called `foo`. There is more information
[in this issue](https://github.com/sorbet/sorbet/issues/3469), which details the
implementation complexity and limitations involved in supporting such a feature.

Code making the most of Sorbet is best written to avoid needing to use
`respond_to?`. Some alternatives include:

- Use [union types](union-types.md) alongside one of the flow-sensitivity
  mechanisms that Sorbet already understands, like `is_a?` or `case`
- Use [interface types](abstract.md) to require that a given interface method
  must exist.

If using `respond_to?` is absolutely necessary, use
[`T.unsafe`](troubleshooting.md#escape-hatches) to call the method after
checking for its existence:

```ruby
sig {params(x: Object).void}
def flow_sensitivity(x)
  if x.respond_to?(:foo)
    # T.unsafe silences all errors from this call site
    T.unsafe(x).foo
  end
end
```

## Flow-sensitivity and the `Singleton` module

Ruby has a module in the standard library called
[`'singleton'`](https://ruby-doc.org/stdlib-3.1.2/libdoc/singleton/rdoc/Singleton.html),
which can be used to create a class that has exactly one instance. Sorbet has
special support for flow-sensitivity on classes that include `Singleton` and
also are marked [`final!`](final.md):

```ruby
# typed: true
extend T::Sig
require 'singleton'

class Unset
  include Singleton
  extend T::Helpers
  final!
end

sig {params(x: T.nilable(T.any(Unset, Integer))).void}
def example1(x: Unset.instance)
  T.reveal_type(x) # => `T.nilable(T.any(Unset, Integer))

  # `==` comparisons on Singleton types update the type in
  # both the `if` and the `else` case:

  if x == Unset.instance
    T.reveal_type(x) # => `Unset`
  else
    T.reveal_type(x) # => `T.nilable(Integer)`
  end
end

sig {params(x: T.nilable(Integer)).void}
def example2(x: nil)
  T.reveal_type(x) # => `T.nilable(Integer)
  if x == 0
    # All `==` comparisons on non-Singleton types only update
    # the type if the type test is true.
    T.reveal_type(x) # => `Integer`
  else
    # When the `==` comparison above is false, the type of `x`
    # remains identical to what it was outside the `if`.
    T.reveal_type(x) # => `T.nilable(Integer)`
  end
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0Arequire%20'singleton'%0A%0Aclass%20Unset%0A%20%20include%20Singleton%0A%20%20extend%20T%3A%3AHelpers%0A%20%20final!%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.nilable%28T.any%28Unset%2C%20Integer%29%29%29.void%7D%0Adef%20example1%28x%3A%20Unset.instance%29%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28T.any%28Unset%2C%20Integer%29%29%0A%0A%20%20%23%20%60%3D%3D%60%20comparisons%20on%20Singleton%20types%20update%20the%20type%20in%0A%20%20%23%20both%20the%20%60if%60%20and%20the%20%60else%60%20case%3A%0A%0A%20%20if%20x%20%3D%3D%20Unset.instance%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60Unset%60%0A%20%20else%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28Integer%29%60%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.nilable%28Integer%29%29.void%7D%0Adef%20example2%28x%3A%20nil%29%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28Integer%29%0A%20%20if%20x%20%3D%3D%200%0A%20%20%20%20%23%20All%20%60%3D%3D%60%20comparisons%20on%20non-Singleton%20types%20only%20update%0A%20%20%20%20%23%20the%20type%20if%20the%20type%20test%20is%20true.%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60Integer%60%0A%20%20else%0A%20%20%20%20%23%20When%20the%20%60%3D%3D%60%20comparison%20above%20is%20false%2C%20the%20type%20of%20%60x%60%0A%20%20%20%20%23%20remains%20identical%20to%20what%20it%20was%20outside%20the%20%60if%60.%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28Integer%29%60%0A%20%20end%0Aend)

As mentioned in the example above, normally `==` only gives additional,
flow-sensitive information about the type of a variable in the case that the
type test was truthy.

But as we see within the `example1` method above, using `==` on a `Singleton`
value will allow Sorbet to update its knowledge about the type of `x` both when
the `==` comparison is true and when it is false.

As seen shown in the example above, this technique can be useful to distinguish
between cases when a possibly-`nil`, optional argument was explicitly passed at
the call site and set to `nil`, or when a value was omitted at the call site and
the default value of `Unset.instance` was used.

Note that using `final!` is required, as without it, the `==` comparison could
return `true` in the presence of subclasses of `Singleton` classes.

[→ Example of `Singleton` but not `final!`](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0Arequire%20'singleton'%0A%0Aclass%20Unset%0A%20%20include%20Singleton%0A%20%20extend%20T%3A%3AHelpers%0Aend%0A%0Aclass%20UnsetChild%20%3C%20Unset%3B%20end%0A%0Asig%20%7Bparams%28x%3A%20T.nilable%28T.any%28Unset%2C%20Integer%29%29%29.void%7D%0Adef%20example1%28x%3A%20Unset.instance%29%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28T.any%28Unset%2C%20Integer%29%29%0A%0A%20%20if%20x%20%3D%3D%20Unset.instance%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60Unset%60%0A%20%20else%0A%20%20%20%20%23%20Can%20still%20reach%20here%20because%20any%20number%20of%20child%20classes%0A%20%20%20%20%23%20of%20%60Unset%60%20could%20also%20exist%2C%20so%20Sorbet%20has%20to%20think%20that%0A%20%20%20%20%23%20the%20type%20could%20still%20include%20%60Unset%60%0A%20%20%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60T.nilable%28T.any%28Unset%2C%20Integer%29%29%0A%20%20end%0Aend%0A%0Aexample1%28x%3A%20UnsetChild.instance%29)

## Limitations of flow-sensitivity

An alternative title for this section: "_Why does Sorbet think this is nil? I
just checked that it's not!_"

Flow-sensitive type checking only works for local variables, not for values
returned from method calls. Why? Sorbet can't know that if a method is called
twice in a row that it returns the same thing each time. Put another way, Sorbet
never assumes that a method call is **pure**.

For example, consider that we have some method `maybe_int` which when called
either returns an Integer or `nil`. This code doesn't typecheck:

```ruby
x = !maybe_int.nil? && (2 * maybe_int)
```

This problem is subtle because `maybe_int` looks like a variable when it's
actually a method! Things become more clear if we rewrite that last line like
this:

```ruby
# This is the same as above:
x = !maybe_int().nil? && (2 * maybe_int())
```

Sorbet can’t know that two calls to `maybe_int` return identical things because,
in general, methods are not pure. The solution is to store the result of the
method call in a **temporary variable**:

```ruby
tmp = maybe_int
y = !tmp.nil? && (2 * tmp)
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Breturns(T.nilable(Integer))%7D%0Adef%20maybe_int%3B%201%3B%20end%0A%0A%23%20Problem%3A%0Ax%20%3D%20!maybe_int.nil%3F%20%26%26%20(2%20*%20maybe_int)%0A%0A%23%20%5E%20this%20is%20essentially%3A%0A%23%20x%20%3D%20!maybe_int().nil%3F%20%26%26%20(2%20*%20maybe_int())%0A%0A%23%20Solution%3A%0Atmp%20%3D%20maybe_int%0Ay%20%3D%20!tmp.nil%3F%20%26%26%20(2%20*%20tmp)">→
View full example on sorbet.run</a>

> **Note**: Many Ruby constructs that look like local variables are actually
> method calls without parens! Specifically, watch out for `attr_reader` and
> zero-argument method definitions.
