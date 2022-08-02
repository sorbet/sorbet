---
id: intersection-types
title: Intersection Types (T.all)
---

Intersection types are how we overlap two types, declaring that an expression
has the all properties of two or more types. The basic syntax for `T.all` is:

```ruby
T.all(Type1, Type2, ...)
```

Note that `T.all` requires at least two arguments.

Unlike union types (`T.any(Type1, Type2)`) which say that an expression is
_either_ `Type1` _or_ the unrelated `Type2`, `T.all(Type1, Type2)` says that an
expression has both of these two types at the same time. Some common use cases
for intersection types:

- Overlapping two otherwise unrelated interfaces, like
  `T.all(Enumerable, Comparable)`.
- Placing a constraint on a method's generic type parameter, like
  `T.all(T.type_parameter(:U), Comparable)`. (For more, see
  [Placing bounds on generic methods](generics.md#placing-bounds-on-generic-methods).)
- Modeling [control flow-sensitive](flow-sensitive.md) type tests on certain
  kinds of values (discussed below).

As the names indicate, union types and intersection types can be understood by
analogy to the related operations on sets. Given `Type1` and `Type2`:

- `T.any(Type1, Type2)` is the union of the set of values in both types: values
  that are either of `Type1`, or of `Type2`.
- `T.all(Type1, Type2)` is the intersection of the set of values in both types:
  values that are of both `Type1` and `Type2`.

An example to make things more concrete:

```ruby
# typed: true
extend T::Sig

module Type1
  def method1; end
end

module Type2
  def method2; end
end

class ExampleClass1
  include Type1
  include Type2
end

class ExampleClass2
  include Type1
end

sig {params(x: T.all(Type1, Type2)).void}
def requires_both(x)
  x.method1 # ok
  x.method2 # also ok
end

requires_both(ExampleClass1.new) # ok
requires_both(ExampleClass2.new) # NOT ok, has Type1 but not Type2
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Amodule%20Type1%0A%20%20def%20method1%3B%20end%0Aend%0A%0Amodule%20Type2%0A%20%20def%20method2%3B%20end%0Aend%0A%0Aclass%20ExampleClass1%0A%20%20include%20Type1%0A%20%20include%20Type2%0Aend%0A%0Aclass%20ExampleClass2%0A%20%20include%20Type1%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.all%28Type1%2C%20Type2%29%29.void%7D%0Adef%20requires_both%28x%29%0A%20%20x.method1%20%23%20ok%0A%20%20x.method2%20%23%20also%20ok%0Aend%0A%0Arequires_both%28ExampleClass1.new%29%20%23%20ok%0Arequires_both%28ExampleClass2.new%29%20%23%20NOT%20ok%2C%20has%20Type1%20but%20not%20Type2)

## Intersection types as ad hoc interfaces

Intersections are useful because they don't require us to write an explicit
interface that combines the requirements of two or more interfaces. For example,
we might have imagined defining something like

```ruby
module Type1AndType2
  include Type1
  include Type2
end
```

and then using `Type1AndType2` instead of `T.all(Type1, Type2)`.

This can sometimes work, but only if we retroactively modify `ExampleClass1` to
be defined like this instead of including the interfaces individually:

```ruby
class ExampleClass1
  include Type1AndType2
end
```

When working with classes and interfaces defined in the standard library, it's
not possible to modify how a class was defined without resorting to monkey
patching the class, which is undesirable.

In this light, `T.all` can be understood as a way to define certain kinds of _ad
hoc_ interfaces.

(Importantly, interfaces do not on their own provide support for "duck typing,"
because the classes in question still have to implement each interface
individually. The only thing that is ad hoc is the combination of two or more
already-implemented interfaces.)

## Understanding how intersection types collapse

In an attempt to both make error messages simpler and make Sorbet type check a
codebase faster, Sorbet attempts to reduce or collapse intersection types to
smaller types. For example:

```ruby
class Parent; end
class Child < Parent; end

sig {params(x: T.all(Parent, Child)).void}
def example(x)
  T.reveal_type(x) # => `Child`
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20Parent%3B%20end%0Aclass%20Child%20%3C%20Parent%3B%20end%0A%0Asig%20%7Bparams%28x%3A%20T.all%28Parent%2C%20Child%29%29.void%7D%0Adef%20example%28x%29%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20%60Child%60%0Aend)

In this example, Sorbet reveals that `x` has type `Child`, despite the method
being declared as taking `T.all(Parent, Child)`.

Given what it knows about which classes are defined and their super classes,
Sorbet is smart enough to realize that every value with type `Child` already
also has type `Parent` via inheritance. That makes `T.all(Child, Parent)`
redundant, and it collapses the type to simply `Child`.

Sorbet is quite smart at collapsing types like this. Some other examples of
types that collapse:

```ruby
# typed: true
extend T::Sig

class A; end
class B; end

module M; end

class FooParent; end
class FooChild < FooParent; end
class Bar; end

module ImmutableBox
  extend T::Generic
  Elem = type_member(:out)
end
class MutableBox
  extend T::Generic
  Elem = type_member
end

sig {params(xs: T::Array[T.all(A, B)]).void}
def example1(xs)
  # Since A and B are unrelated classes, Sorbet notices that
  # there are no values that satisfy `T.all(A, B)`, and thus
  # collapses the type to `T.noreturn`
  T.reveal_type(xs) # => T::Array[T.noreturn]
end

sig {params(x: T.all(A, M)).void}
def example2(x)
  # Even though A and M are unrelated, because M is a module
  # (not a class) the type does not collapse. Why? There might
  # be some subclasses of A that include M, and some that don't.
  #
  # In this example, A has no subclasses. If we explicitly
  # declare to Sorbet that A has no subclasses with `final!`,
  # it would collapse the type.
  T.reveal_type(x) # => T.all(A, M)
end

sig {params(x: T.all(FooParent, T.any(FooChild, Bar))).void}
def example3(x)
  # Sorbet is smart enough to distribute over union types:
  #    T.all(FooParent, T.any(FooChild, Bar))
  # => T.any(T.all(FooParent, FooChild), T.all(FooParent, Bar))
  # => T.any(FooChild                  , T.noreturn           )
  # => FooChild
  T.reveal_type(x) # => FooChild
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20A%3B%20end%0Aclass%20B%3B%20end%0A%0Amodule%20M%3B%20end%0A%0Aclass%20FooParent%3B%20end%0Aclass%20FooChild%20%3C%20FooParent%3B%20end%0Aclass%20Bar%3B%20end%0A%0Amodule%20ImmutableBox%0A%20%20extend%20T%3A%3AGeneric%0A%20%20Elem%20%3D%20type_member%28%3Aout%29%0Aend%0Aclass%20MutableBox%0A%20%20extend%20T%3A%3AGeneric%0A%20%20Elem%20%3D%20type_member%0Aend%0A%0Asig%20%7Bparams%28xs%3A%20T%3A%3AArray%5BT.all%28A%2C%20B%29%5D%29.void%7D%0Adef%20example1%28xs%29%0A%20%20%23%20Since%20A%20and%20B%20are%20unrelated%20classes%2C%20Sorbet%20notices%20that%0A%20%20%23%20there%20are%20no%20values%20that%20satisfy%20%60T.all%28A%2C%20B%29%60%2C%20and%20thus%0A%20%20%23%20collapses%20the%20type%20to%20%60T.noreturn%60%0A%20%20T.reveal_type%28xs%29%20%23%20%3D%3E%20T%3A%3AArray%5BT.noreturn%5D%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.all%28A%2C%20M%29%29.void%7D%0Adef%20example2%28x%29%0A%20%20%23%20Even%20though%20A%20and%20M%20are%20unrelated%2C%20because%20M%20is%20a%20module%0A%20%20%23%20%28not%20a%20class%29%20the%20type%20does%20not%20collapse.%20Why%3F%20There%20might%0A%20%20%23%20be%20some%20subclasses%20of%20A%20that%20include%20M%2C%20and%20some%20that%20don't.%0A%20%20%23%20%0A%20%20%23%20In%20this%20example%2C%20A%20has%20no%20subclasses.%20If%20we%20explicitly%0A%20%20%23%20declare%20to%20Sorbet%20that%20A%20has%20no%20subclasses%20with%20%60final!%60%2C%0A%20%20%23%20it%20would%20collapse%20the%20type.%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20T.all%28A%2C%20M%29%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.all%28FooParent%2C%20T.any%28FooChild%2C%20Bar%29%29%29.void%7D%0Adef%20example3%28x%29%0A%20%20%23%20Sorbet%20is%20smart%20enough%20to%20distribute%20over%20union%20types%3A%0A%20%20%23%20%20%20%20T.all%28FooParent%2C%20T.any%28FooChild%2C%20Bar%29%29%0A%20%20%23%20%3D%3E%20T.any%28T.all%28FooParent%2C%20FooChild%29%2C%20T.all%28FooParent%2C%20Bar%29%29%0A%20%20%23%20%3D%3E%20T.any%28FooChild%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%2C%20T.noreturn%20%20%20%20%20%20%20%20%20%20%20%29%0A%20%20%23%20%3D%3E%20FooChild%0A%20%20T.reveal_type%28x%29%20%23%20%3D%3E%20FooChild%0Aend)

All the examples above use normal, non-generic classes and modules, however the
same principles govern when intersection types involving generic classes and
modules collapse as well.

## Intersection types and control flow

Without even realizing, you use intersection types almost every time you write
code with Sorbet, _indirectly_ via [control flow-sensitive](flow-sensitive.md)
typing. For example:

```ruby
class A; end
class B; end

# Note: `T.any` (either A or B)
sig {params(x: T.any(A, B)).void}
def example(x)
  case x
  when A
    T.reveal_type(x) # => `A`
  end
end
```

In this example, due to flow sensitive typing Sorbet understands that `x` has
type `A` within the `when A` clause. But it doesn't simply _ascribe_ that type
to `x`: it arrives at that type using intersection types. Inside the `when`
clause, Sorbet uses intersection types to say that `x` has "both the type it
used to have and also `A`": `T.all(T.any(A, B), A)`.

But as we know from the previous section, this type collapses:

```ruby
   T.all(T.any(A, B), A)
=> T.any(T.all(A, A), T.all(B, A))
=> T.any(A          , T.noreturn)
=> A
```

We can witness that this is the case by using a control flow type test on an
unrelated module:

```ruby
class A; end

module M; end

sig {params(x: A).void}
def example(x)
  case x
  when M
    T.reveal_type(x) # => `T.all(A, M)`
  end
end
```

In this example, the method signature said nothing about whether or not `x`
implements the module `M`, but in the method body we ask at runtime whether it
does or not. This is enough to let Sorbet know that if the type test were to
pass at runtime and run the code inside the `when` branch, that `x` must now
have all the methods that were defined on `A` _and_ all the methods defined on
`M`. (And as mentioned in the previous section, this type does not collapse
because `A` is not `final!`.)

## What's next?

- [`T.noreturn`](noreturn.md)

  Use of intersection types can lead to (sometimes unwanted) `T.noreturn` types.
  Read more about what `T.noreturn` represents, and when it is useful.

- [`Abstract Classes and Interfaces`](abstract.md)

  Intersection types are most commonly used in tandem with interface modules.
  Read more about how to define composable abstract classes and interfaces.
