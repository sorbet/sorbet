---
id: unknown
title: T.unknown
---

The type `T.unknown` is a type that is a supertype of all other types in Sorbet.
In this sense, it is the "top" type of Sorbet's type system.

```ruby
sig {params(x: T.unknown).returns(T.unknown)}
def example(x)
  x.nil? # error: `nil?` doesn't exist on `T.unknown`
  x
end

example(0)  # ok
example('') # ok
```

In this `example` method the parameter `x` has type `T.unknown`, Sorbet lets it
be called with anything. However, since Sorbet knows nothing about what methods
exist on `T.unknown`, it rejects all methods calls on it (including `.nil?` as a
seen here).

## Doing something with `T.unknown`

`T.unknown` requires being explicitly downcast before it's possible to do
anything meaningful with a value of such a type:

```ruby
sig {params(x: T.unknown).void}
def print_if_even(x)
  x.even? # error: Don't know whether `x` is an `Integer`

  # option 1: safe downcast
  case x
  when Integer
    if x.even?
      puts("it's even")
    else
      puts("it's not even")
    end
  else
    # ... handle case when not an `Integer` ...
  end

  # option 2: unchecked downcast
  y = T.cast(x, Integer) # will raise at runtime if not an `Integer`
  if y.even?
    puts("it's even")
  else
    puts("it's not even")
  end
end
```

In option 1, we use `case` to check whether `x` is an `Integer`, which makes it
easy to handle the case when `x` is not an `Integer`, too. Note that we have to
use `case` and not `is_a?`, because `is_a?` is a method, and `T.unknown` does
not respond to any methods.

In option 2, we use `T.cast` to do a [runtime-only cast](type-assertions.md) to
raise an exception if `x` is not an `Integer` at runtime.

Viewed like this, `T.unknown` is a kind of forcing mechanism to require that
consumers of some otherwise "untyped" interface do runtime type checks to verify
that the type is what they expect.

## `T.unknown` vs `T.untyped`

Despite having similar names, `T.unknown` is not the same as `T.untyped`:

- `T.unknown` is a supertype of all other types, **but** is not a subtype of any
  other type (except itself).

- `T.untyped` is a super type of all other types, **and** is a subtype of all
  other types (which is a contradiction that lies at the core of a
  [gradual type system](gradual.md)).

To drive the difference home:

```ruby
sig {params(x: Integer).void}
def takes_integer(x); end

sig do
  params(
    something_untyped: T.untyped,
    something_unknown: T.unknown,
  )
  .void
end
def example(something_unknown, something_untyped)
  # OK, because `T.untyped` is a subtype of everything
  takes_integer(something_untyped)

  # NOT OK, because T.unknown is only a subtype of `T.unknown`,
  # not `Integer` nor anything else
  takes_integer(something_unknown)
end
```

## `T.unknown` vs `BasicObject`

[`BasicObject`] is somewhat similar to `T.unknown`. In Ruby, `BasicObject` is
the parent class of all classes. But `T.unknown` is an even wider type than
`BasicObject`.

[`basicobject`]: class-types.md

The distinction is subtle but important. For example, maybe we want to build an
[interface](abstract.md) that only exposes a single method:

```ruby
module IFoo
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end
end

sig {params(x: IFoo, y: IFoo).void}
def example(x, y)
  x.foo # obviously okay

  x == y # should this be okay? (spoiler: it's not)
end
```

In this example: it's totally fine to call `x.foo`, because our `IFoo` interface
exposes a `foo` method. But should the call to `x == y` be allowed?

The `==` method isn't in our interface. Technically speaking, `BasicObject`
defines `==` for all objects, but it's not necessarily the case that it makes
sense to compare all things that implement the `IFoo` interface. As the author
of this interface, we might actually **want** to have Sorbet tell us when we're
calling a method that's not in the interface.

For this reason, Sorbet does not treat `IFoo` as a subtype of `BasicObject`.
Programmers are free to build precisely the interface they'd like to expose, and
never have to worry about "hiding" the methods from `BasicObject` that they
don't want to expose.

This is why `T.unknown` is useful: there is still _some_ type to write down in
cases where truly passing in anything or returning anything is fine.

## `T.unknown` vs `T.type_parameter(:U)`

Another common way to declare that a method "accepts anything" is to use a
[generic method](generics.md):

```ruby
sig {params(x: T.unknown).void}
def takes_anything_unknown(x)
  takes_anything_generic(x) # okay
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def takes_anything_generic(x)
  takes_anything_unknown(x) # okay
end
```

There are some subtle differences between these approaches, but overall they're
quite similar. In fact, the body of `takes_anything_generic` is allowed to pass
`x`, which has type `T.type_parameter(:U)` to `takes_anything_unknown`. The
opposite calling direction works as well.

So how are they different? Whenever using `T.type_parameter(:U)` in a method
signature, **all** occurrences of the type have to agree. In a method like the
identify function, that means that the output has to be verbatim something that
was provided as input:

```ruby
sig {params(x: T.unknown).returns(T.unknown)}
def f(x)
  # ...
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T.type_parameter(:U))
end
def identity(x)
  res = f(x)
  return res # error!
end
```

The signature for `f` says that it takes `T.unknown` and returns `T.unknown`,
but it is not required that the thing it returns is at all related to the thing
it took as input.

Meanwhile, the signature for `identity` says that it returns exactly what it was
given as input. As such, it's an error in the snippet above to have `identity`
return `f(x)` instead of simply `x`.

But for methods that only mention a given generic type parameter once (like our
`takes_anything_unknown` and `takes_anything_generic` methods above),
`T.unknown` and `T.type_parameter(:U)` are nearly indistinguishable.
