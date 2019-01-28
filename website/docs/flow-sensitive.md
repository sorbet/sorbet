---
id: flow-sensitive
title: Flow-Sensitive Typing
sidebar_label: Flow Sensitivity
---

Sorbet implements a **control flow sensitive** type system. It models control
flow through a program and uses it to track the program's types more
accurately.[^control]

[^control]: We abbreviate "control flow sensitive" to "flow-sensitive"
throughout these docs, because Sorbet does little to no *data flow analysis*.
(Data flow analysis is a separate family of techniques that models the way data
flows between variables in a program.)

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
- `Class#===` (this is how `case` on a class object works)
- `Class#<` (like `is_a?`, but for class objects instead of instances of
- Negated conditions (including both `!` and `unless`)
- Truthiness (everything but `nil` and `false` is truthy in Ruby)
  classes)

> **Warning**: that Sorbet's analysis for these constructs hinges on them not
> being overridden! For example, Sorbet can behave unpredictably if when
> overriding `is_a?` in weird ways.

## Limitations of flow-sensitivity

An alternative title for this section: "*Why does Sorbet think this is nil? I
just checked that it's not!*"

Flow-sensitive type checking only works for local variables, not for values
returned from method calls. Why? Sorbet can't know that if a method is called
twice in a row that it returns the same thing each time. Put another way, Sorbet
never assumes that a method call is **pure**.

For example, consider that we have some method `maybe_int` which when called
either returns an Integer or `nil`. This code doesn't typecheck:

```ruby
x = maybe_int.nil? && (2 * maybe_int)
```

This problem is subtle because `maybe_int` looks like a variable when it's
actually a method! Things become more clear if we rewrite that last line like
this:

```ruby
# This is the same as above:
x = maybe_int().nil? && (2 * maybe_int())
```

Sorbet can’t know that two calls to `maybe_int` return identical things because
in general methods are not pure. The solution is to store the result of the
method call in a **temporary variable**:

```ruby
tmp = maybe_int
y = tmp.nil? && (2 * tmp)
```

<a href="https://sorbet.run/#extend%20T%3A%3ASig%0A%0Asig%20%7Breturns(T.nilable(Integer))%7D%0Adef%20maybe_int%3B%201%3B%20end%0A%0A%23%20Problem%3A%0Ax%20%3D%20maybe_int.nil%3F%20%26%26%20(2%20*%20maybe_int)%0A%0A%23%20%5E%20this%20is%20essentially%3A%0A%23%20x%20%3D%20maybe_int().nil%3F%20%26%26%20(2%20*%20maybe_int())%0A%0A%23%20Solution%3A%0Atmp%20%3D%20maybe_int%0Ay%20%3D%20tmp.nil%3F%20%26%26%20(2%20*%20tmp)">→ View full example on sorbet.run</a>

> **Note**: many Ruby constructs that look like local variables are actually
> method calls without parens! Specifically, watch out for `attr_reader` and
> zero-argument method definitions.

