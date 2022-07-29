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
