---
id: overloads
title: Methods with Overloaded Signatures
sidebar_label: Overloads
---

Sorbet has minimal support for defining methods with overloaded signatures.

```ruby
sig { params(x: Integer).returns(Integer) }
sig { params(x: String).returns(String) }
def example(x); end
```

## Consider not using overloads

Overloads have multiple downsides:

- They encourage unwieldy signatures, which are confusing to understand.
- They aren't as precise in [gradually typed](gradual.md) languages like Sorbet
  as they are in typed languages which lack [`T.untyped`](untyped.md).
- Their implementation in Sorbet is somewhat second class (due in part to the
  previous two points).

Instead of using overloaded methods, consider simply defining multiple methods.

For example, instead of defining a method like the following, which accepts
either a string or an array of strings and returns either a single `MyModel` or
an array of `MyModel`s:

```ruby
sig { params(what: String).returns(MyModel) }
sig { params(what: T::Array[String]).returns(T::Array[MyModel]) }
def find(what); end
```

Consider instead simply defining two methods, each with a descriptive name:

```ruby
sig { params(id: String).returns(MyModel) }
def find_one(id); end

sig { params(ids: T::Array[String]).returns(T::Array[MyModel]) }
def find_many(ids); end
```

The benefits of this approach:

- Each method has a descriptive name, making the meaning more well-understood at
  the call site.
- The argument names can be unique in each definition as well. In this example:
  `id` vs `ids`.
- Each method can be documented independently. Documentation above a method
  definition is surfaced when hovering over a method call and when selecting
  completion items.
- Since the method name controls which method is selected, untyped arguments do
  not interfere with the inferred return type of a method call.

### Multiple methods, but sharing a common implementation

The biggest downside of this approach has to do with sharing code.

In most cases, it's possible to implement one method in terms of the other. For
example, with our `find_one`/`find_many` example above:

```ruby
sig { params(id: String).returns(MyModel) }
def find_one(id)
  result = find_many([id])
  raise "find_many did not return a single result" unless result.size == 1
  result.fetch(0)
end
```

In cases when this is not possible, another option is to do something like this:

```ruby
sig do
  params(what: T.any(String, T::Array[String]))
    .returns(T.any(MyModel, T::Array[MyModel]))
end
private def _find_impl
  # ...
end

sig { params(id: String).returns(MyModel) }
def find_one(id)
  T.unsafe(_find_impl(id))
end

sig { params(ids: T::Array[String]).returns(T::Array[MyModel]) }
def find_many(ids)
  T.unsafe(_find_impl(ids))
end
```

In this example, we define the common logic in a private `_find_impl` method
with a signature that accepts the superset of all arguments, and returns a
superset of all return types. We then call that method inside public methods
with more specific types, using `T.unsafe`. This keeps usage of `T.untyped`
internal to the class's private implementation while exposing a typed public
API.

There are a couple other ways to accomplish a similar effect:

- Mark the return type of `_find_impl` as `T.untyped`, to avoid needing a
  `T.unsafe` at each call site. Or omit the signature on `_find_impl` entirely.

- Define the signatures for `find_one` and `find_many` in an [RBI file](rbi.md)
  alongside the source file, and use something like

  ```ruby
  T.unsafe(self).alias_method(:find_one, :_find_impl)
  T.unsafe(self).alias_method(:find_many, :_find_impl)
  ```

  to create method aliases to the `_find_impl` method, but using `T.unsafe` to
  hide those aliases from Sorbet, so that the RBI definitions are all Sorbet
  sees.

Any of the options presented here will offer a more first-class experience than
attempting to define a method with overloaded signatures.

## Restrictions on overloaded methods

Support for overloaded methods is minimal because there are restrictions on when
they are allowed to appear and how they are allowed to be used.

Overloaded signatures:

1.  may only appear in RBI files, not in Ruby source files.

1.  prevent the implementation of that method from being type checked.

    If Sorbet sees the method in a source file (in addition to the overloaded
    definition in an RBI file), it **will not type check** the method's body. In
    fact, Sorbet will report an error in `# typed: true` files or higher when
    this happens.

    It's expected that overloaded methods are only used to type external gems'
    methods, which can't be rewritten to avoid overloads using the techniques
    mentioned in the
    [Consider not using overloads](#consider-not-using-overloads) section.

1.  are scanned top-to-bottom when attempting to select a suitable overload.

    If no suitable overload is found, the first overload is selected, which may
    be wrong. If multiple suitable overloads are found, the first suitable
    candidate is used.

1.  use a very simplistic, sometimes-wrong heuristic for selecting an overload.

    Some examples of the limitations of this heuristic:

    - Overloads which are [generic methods](generics.md), only get approximate
      constraint resolution, which means that Sorbet may select an overload
      optimistically that causes errors when it could have picked another
      overload without errors.

    - Exactly one overload candidate is selected in the end. More specifically,
      Sorbet will never attempt to select two overloads and merge their results
      together. (For example, if there is an overload which accepts `String`
      arguments, and another which accepts `Integer` arguments, when passed an
      argument of type `T.any(Integer, String)` Sorbet will find no suitable
      overload, and default to the first, which only accepts `Integer`.)

      The usual workaround for cases like these is to manually declare a final
      overload which accepts a superset of all types that the method should be
      able to accept.

1.  are thwarted by untyped arguments.

    In the presence of untyped arguments, chances are high that the first
    overload is selected, which might not be desired.

1.  do not support using the presence, absence, or types of keyword parameters
    for deciding whether a given overload is selected.

### Why these restrictions?

Consider how overloading works in typed, compiled languages like C++ or Java:
each overload is a separate method. They actually have separate implementations,
are type checked separately, compile (with link-time name mangling) to separate
symbols in the compiled object, and the compiler knows how to resolve each call
site to a specific overload ahead of time, either statically or dynamically via
virtual dispatch.

Meanwhile, Ruby itself doesn't have overloading—there's only ever one method
registered with a given name in the VM, regardless of what parameters it
accepts. That complicates things. It becomes unclear how Sorbet should typecheck
the body of the method (against all sigs? against one sig? against the
component-wise union of their arguments?). There's no clear answer, and anything
we choose will be bound to confuse or surprise someone.

Also because Sorbet doesn't control whether the method can be dispatched to,
even if it were going to make a static claim about whether the code type checks,
it doesn't get to control which (fake) overload will get dispatched to at the
call site (again: there's only one version of the method in the VM).

Finally this choice is somewhat philosophical: codebases that make heavy use of
overloading (even in typed languages where overloading is supported) tend to be
harder for readers to understand at a glance. The above workaround of defining
multiple methods with unique names solves this readability problem, because now
each overload has a descriptive name.

## Defining methods with overloaded signatures

Unlike other methods, where every parameter in the method definition must also
have a type in the signature, overloaded signatures are allowed to omit
parameters. For example:

```ruby
sig { returns(Enumerator[Integer]) }
sig { params(blk: T.proc.params(x: Integer).void).void }
def example(&blk); end

x = example
T.reveal_type(x) # => Enumerator[Integer]

y = example { |x| p(x) }
T.reveal_type(y) # => void
```

Notice how the first overloaded signature omits giving a type for the `blk`
parameter. Sorbet uses the presence or absence of an argument (including a block
argument) to select a suitable overload.
