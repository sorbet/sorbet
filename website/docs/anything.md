---
id: anything
title: T.anything
---

The type `T.anything` is a type that is a supertype of all other types in
Sorbet. In this sense, it is the "top" type of Sorbet's type system.

```ruby
sig {params(x: T.anything).returns(T.anything)}
def example(x)
  x.nil? # error: `nil?` doesn't exist on `T.anything`
  x
end

example(0)  # ok
example('') # ok
```

In this `example` method the parameter `x` has type `T.anything`, Sorbet lets it
be called with anything. However, since Sorbet knows nothing about what methods
exist on `T.anything`, it rejects all methods calls on it (including `.nil?` as
a seen here).

## Doing something with `T.anything`

`T.anything` requires being explicitly downcast before it's possible to do
anything meaningful with a value of such a type:

```ruby
sig {params(x: T.anything).void}
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
use `case` and not `is_a?`, because `is_a?` is a method, and `T.anything` does
not respond to any methods.

In option 2, we use `T.cast` to do a [runtime-only cast](type-assertions.md) to
raise an exception if `x` is not an `Integer` at runtime.

Viewed like this, `T.anything` is a kind of forcing mechanism to require that
consumers of some otherwise "untyped" interface do runtime type checks to verify
that the type is what they expect.

## `T.anything` vs `T.untyped`

`T.anything` is not the same as `T.untyped`:

- `T.anything` is a supertype of all other types, **but** is not a subtype of
  any other type (except itself).

- `T.untyped` is a super type of all other types, **and** is a subtype of all
  other types (which is a contradiction that lies at the core of a
  [gradual type system](gradual.md)).

In simpler terms, Sorbet essentially assumes that a `T.untyped` value is being
used correctly. But for `T.anything`, Sorbet does not allow treating it as if it
were a specific type without some sort of runtime type check or cast.

To drive the difference home:

```ruby
sig {params(x: Integer).void}
def takes_integer(x); end

sig do
  params(
    something_untyped: T.untyped,
    could_be_anything: T.anything,
  )
  .void
end
def example(could_be_anything, something_untyped)
  # OK, because `T.untyped` is a subtype of everything
  takes_integer(something_untyped)

  # NOT OK, because `T.anything` is only a subtype of `T.anything`,
  # not `Integer` nor anything else
  takes_integer(could_be_anything)
end
```

## `T.anything` vs `BasicObject`

[`BasicObject`] is somewhat similar to `T.anything`. In Ruby, `BasicObject` is
the parent class of all classes. But `T.anything` is an even wider type than
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

This is why `T.anything` is useful: there is still _some_ type to write down in
cases where truly passing in anything or returning anything is fine.

## `T.anything` vs `T.type_parameter(:U)`

Another common way to declare that a method "accepts anything" is to use a
[generic method](generics.md):

```ruby
sig {params(x: T.anything).void}
def takes_anything(x)
  takes_anything_generic(x) # okay
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def takes_anything_generic(x)
  takes_anything(x) # okay
end
```

There are some subtle differences between these approaches, but overall they're
quite similar. In fact, the body of `takes_anything_generic` is allowed to pass
`x`, which has type `T.type_parameter(:U)` to `takes_anything`. The opposite
calling direction works as well.

So how are they different? Whenever using `T.type_parameter(:U)` in a method
signature, **all** occurrences of the type have to agree. In a method like the
identify function, that means that the output has to be verbatim something that
was provided as input:

```ruby
sig {params(x: T.anything).returns(T.anything)}
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

The signature for `f` says that it takes `T.anything` and returns `T.anything`,
but it is not required that the thing it returns is at all related to the thing
it took as input.

Meanwhile, the signature for `identity` says that it returns exactly what it was
given as input. As such, it's an error in the snippet above to have `identity`
return `f(x)` instead of simply `x`.

But for methods that only mention a given generic type parameter once (like our
`takes_anything` and `takes_anything_generic` methods above), `T.anything` and
`T.type_parameter(:U)` are nearly indistinguishable.

## `T.anything` and RBIs

Historically, Sorbet has favored being easy to adopt over avoiding `T.untyped`.
This means that certain methods, for example `JSON.parse`, have been declared to
return `T.untyped` instead of `T.anything` (or something more specific).

While we're not opposed to using `T.anything` in more places in RBI files, in
each case we will judge it's value against how costly it would be to adopt the
RBI change. See
[the FAQ](faq.md#it-looks-like-sorbets-types-for-the-stdlib-are-wrong) for more
information about contributing RBI improvements.
