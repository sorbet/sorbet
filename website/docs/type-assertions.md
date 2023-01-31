---
id: type-assertions
title: Type Assertions
sidebar_label: T.let, T.cast, T.must, T.bind
---

There are five ways to assert the types of expressions in Sorbet:

- `T.let(expr, Type)`
- `T.cast(expr, Type)`
- `T.must(expr)` / `T.must_because(expr) {msg}`
- `T.assert_type!(expr, Type)`
- `T.bind(self, Type)`

> There is also `T.unsafe` which is not a "type assertion" so much as an
> [Escape Hatch](troubleshooting.md#escape-hatches).

## `T.let`

A `T.let` assertion is checked statically **and** at runtime. In the following
example, the definition of `y` will raise an error when Sorbet is run, and also
when the program is run.

```ruby
x = T.let(10, Integer)
T.reveal_type(x) # Revealed type: Integer

y = T.let(10, String) # error: Argument does not have asserted type String
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Ax%20%3D%20T.let(10%2C%20Integer)%0AT.reveal_type(x)%20%23%20Revealed%20type%3A%20Integer%0A%0Ay%20%3D%20T.let(10%2C%20String)%20%23%20error%3A%20Argument%20does%20not%20have%20asserted%20type%20String">
  → View on sorbet.run
</a>

At runtime, a `TypeError` will be raised when the assignment to `y` is
evaluated:

```cli
$ ruby test.rb
<...>/lib/types/private/casts.rb:15:in `cast': T.let: Expected type String, got type Integer with value 10 (TypeError)
Caller: test.rb:8
	from <...>/lib/types/_types.rb:138:in `let'
	from test.rb:8:in `<main>'
```

## `T.cast`

Sometimes we the programmer are aware of an invariant in the code that isn't
currently expressible in the Sorbet type system:

```ruby
extend T::Sig

class A; def foo; end; end
class B; def bar; end; end

sig {params(label: String, a_or_b: T.any(A, B)).void}
def foo(label, a_or_b)
  case label
  when 'a'
    a_or_b.foo
  when 'b'
    a_or_b.bar
  end
end
```

In this case, we know (through careful test cases / confidence in our production
monitoring) that every time this method is called with `label = 'a'`, `a_or_b`
is an instance of `A`, and same for `'b'` / `B`.

Ideally we'd refactor the code to express this invariant in the types. To
reiterate: the **preferred** solution is to refactor this code. The time spent
adjusting this code now will make it easier and safer to refactor the code in
the future. Even still, we don't always have the time _right now_, so let's see
how we can work around the issue.

We can use `T.cast` to explicitly tell our invariant to Sorbet:

```ruby
  case label
  when 'a'
    T.cast(a_or_b, A).foo
  when 'b'
    T.cast(a_or_b, B).bar
  end
```

Sorbet cannot **statically** guarantee that a `T.cast`-enforced invariant will
succeed in every case, but it will check the invariant **dynamically** on every
invocation.

`T.cast` is better than `T.unsafe`, because it means that something like

```ruby
    T.cast(a_or_b, A).bad_method
```

will still be caught as a missing method statically.

## `T.must`

<a id="tmust_because"></a>

`T.must` is for asserting that a value of a [nilable type](nilable-types.md) is
not `nil`. `T.must` is similar to `T.cast` in that it will not necessarily
trigger an error when `srb tc` is run, but can trigger an error during runtime.

`T.must_because` is like `T.must` but also takes a reason why the value is not
expected to be `nil`, which appears in the exception that is raised if passed a
`nil` argument.

The following example illustrates two cases:

1. a use of `T.must` with a value that Sorbet is able to determine statically is
   `nil`, that raises an error indicating that the subsequent statements are
   unreachable;
2. a use of `T.must` with a computed `nil` value that Sorbet is not able to
   detect statically, which raises an error at runtime.

```ruby
class A
  extend T::Sig

  sig {void}
  def foo
    x = T.let(nil, T.nilable(String))
    y = T.must(nil)
    puts y # error: This code is unreachable
  end

  sig {void}
  def bar
    vals = T.let([], T::Array[Integer])
    x = vals.find {|a| a > 0}
    T.reveal_type(x) # Revealed type: T.nilable(Integer)
    y = T.must(x)
    puts y # no static error
  end

end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20foo%0A%20%20%20%20x%20%3D%20T.let(nil%2C%20T.nilable(String))%0A%20%20%20%20y%20%3D%20T.must(nil)%0A%20%20%20%20puts%20y%20%23%20error%3A%20This%20code%20is%20unreachable%0A%20%20end%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20bar%0A%20%20%20%20vals%20%3D%20T.let(%5B%5D%2C%20T%3A%3AArray%5BInteger%5D)%0A%20%20%20%20x%20%3D%20vals.find%20%7B%7Ca%7C%20a%20%3E%200%7D%0A%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20T.nilable(Integer)%0A%20%20%20%20y%20%3D%20T.must(x)%0A%20%20%20%20puts%20y%20%23%20no%20static%20error%0A%20%20end%0A%0Aend">
  → View on sorbet.run
</a>

Here's the same example with `T.must_because`, showing the user of custom
reasons. The reason is provided as a block that returns a `String`, so that the
reason is only computed if the exception would be raised.

```ruby
class A
  extend T::Sig

  sig {void}
  def foo
    y = T.must_because(nil) {'reason'}
    puts y # error: This code is unreachable
  end

  sig {void}
  def bar
    vals = T.let([], T::Array[Integer])
    x = vals.find {|a| a > 0}
    T.reveal_type(x) # Revealed type: T.nilable(Integer)
    y = T.must_because(x) {'reason'}
    puts y # no static error
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20foo%0A%20%20%20%20y%20%3D%20T.must_because%28nil%29%20%7B'reason'%7D%0A%20%20%20%20puts%20y%20%23%20error%3A%20This%20code%20is%20unreachable%0A%20%20end%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20bar%0A%20%20%20%20vals%20%3D%20T.let%28%5B%5D%2C%20T%3A%3AArray%5BInteger%5D%29%0A%20%20%20%20x%20%3D%20vals.find%20%7B%7Ca%7C%20a%20%3E%200%7D%0A%20%20%20%20T.reveal_type%28x%29%20%23%20Revealed%20type%3A%20T.nilable%28Integer%29%0A%20%20%20%20y%20%3D%20T.must_because%28x%29%20%7B'reason'%7D%0A%20%20%20%20puts%20y%20%23%20no%20static%20error%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

## `T.assert_type!`

`T.assert_type!` is similar to `T.let`: it is checked statically **and** at
runtime. It has the additional restriction that it will **always** fail
statically if given something that's [`T.untyped`](untyped.md). For example:

```ruby
class A
  extend T::Sig

  sig {params(x: T.untyped).void}
  def foo(x)
    T.assert_type!(x, String) # error here
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.untyped).void%7D%0A%20%20def%20foo(x)%0A%20%20%20%20T.assert_type!(x%2C%20String)%20%23%20error%20here%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

## `T.bind`

`T.bind` works like `T.cast`, except with special syntactic sugar for `self`.
Like `T.cast`, it is unchecked statically but checked at runtime. Unlike
`T.cast`, it does not require assigning the result to a variable.

Sometimes we would like to use `T.cast` to ascribe a type for `self`. One option
is to assign the cast result to a variable, perhaps called `this`:

```ruby
this = T.cast(self, MyClass)
this.method_on_my_class
```

This is annoying:

- It requires replacing `self` with `this` everywhere it's used.
- It prevents calling private methods.

If we tried to clean this up with something like `self = T.cast(self, ...)`, the
Ruby VM rejects our code with a syntax error: `self` is not a variable, and
can't be used as the name of one.

Thus, Sorbet provides `T.bind` for this specific usecase instead:

```ruby
T.bind(self, MyClass)
self.method_on_my_class
```

`T.bind` is the only type assertion that does not require assigning the
assertion result into a variable, and it can only be used on `self`.

`T.bind` can be used anywhere `self` is used (i.e., methods, blocks, lambdas,
etc.), though it is most usually useful within blocks. See
[Blocks, Procs, and Lambda Types](procs.md) for more real-world usage examples.

## Static vs Runtime Checking

At runtime, all of these assertions verify the `expr` they are passed matches
the `Type` they are passed.

Statically, e.g., when type checking with `srb tc`, some of them are **assumed**
to hold, but not statically checked.

| Assertion                    | Static      | Runtime |
| ---------------------------- | ----------- | ------- |
| `T.let(expr, Type)`          | checked     | checked |
| `T.cast(expr, Type)`         | **assumed** | checked |
| `T.must(expr)`               | **assumed** | checked |
| `T.assert_type!(expr, Type)` | checked     | checked |
| `T.bind(self, Type)`         | **assumed** | checked |

When an assertion is assumed to hold statically, Sorbet will only use it for the
purpose of updating its internal understanding of the types, and will never
attempt to alert the programmer that an assumption might not hold. In this
sense, those assertions can be considered
[Escape Hatches](troubleshooting.md#escape-hatches) for getting something to
typecheck that might not otherwise.

**Note** that even though all of these assertions are checked at runtime, some
**individual types** might never be checked at runtime, regardless of the type
assertion used. This includes the element types of generics (like the `Integer`
in `T::Array[Integer]`), the argument and return types of
[Proc Types](procs.md), [`T.self_type`](self-type.md),
[`T.attached_class`](attached-class), and others.

These assertions are also subject to the `T::Configuration` hooks that
`sorbet-runtime` provides for controlling runtime type checking. See
[Runtime Configuration](tconfiguration.md) for more. By default, all of these
assertions will raise a `TypeError` if they are violated at runtime.

It's possible to opt out of runtime checking for individual calls to `T.let`,
`T.cast`, and `T.bind` by adding `checked: false`, e.g.
`x = T.let(y, Foo, checked: false)`. This isn't recommended in most
circumstances, even in performance-critical code; while adding `checked(:never)`
to a method signature is an easy way to remove performance overhead, doing the
same for `T.let` removes neither the method call overhead nor the overhead of
constructing any type argument. For more effective options, see below.

## Comparison of type assertions

Here are some other ways to think of the behavior of the individual type
assertions:

- `T.let` vs `T.cast`

  ```ruby
  T.cast(expr, Type)
  ```

  is the same as

  ```ruby
  T.let(T.unsafe(expr), Type)
  ```

- `T.unsafe` in terms of `T.let`

  ```ruby
  T.unsafe(expr)
  ```

  is the same as

  ```ruby
  T.let(expr, T.untyped)
  ```

- `T.must` is like `T.cast`, but without having to know the result type:

  ```ruby
  T.cast(nil_or_string, String)
  ```

  is the same as

  ```ruby
  T.must(nil_or_string)
  ```

- `T.bind` is like `T.cast`, but only for `self`,

  ```ruby
  T.bind(self, String)
  ```

  behaves like

  ```ruby
  self = T.cast(self, String)
  ```

  if it were valid in Ruby to assign to `self`.

## Performance considerations

Unlike `sig` annotations, type assertions _always_ have a performance cost, even
if runtime checks are globally disabled or `checked: false` is used at
individual callsites. `T.let` and friends are ordinary Ruby method calls, which
have intrisic overhead, in addition to the overhead of constructing any type
arguments.

This overhead isn't normally worth worrying about, but in code where you are
already micro-optimizing to reduce method calls or object allocations, there are
a few patterns that may be helpful:

### Prefer method signatures over type assertions

It's often possible to avoid using a type assertion at all, without loss of type
safety, with a slight refactoring.

For example, one common use for a type assertion is defining the type of an
instance variable. One can frequently move this type definition to the signature
of the constructor, taking advantage of Sorbet's ability to infer instance
variable types when variables are set directly from constructor arguments.

Instead of:

```ruby
sig {void.checked(:tests)}
def initialize
  @foo = T.let(MyObject.new, MyInterface)
end
```

Write:

```ruby
sig {params(foo: MyInterface).void.checked(:tests)}
def initialize(foo: MyObject.new)
  @foo = foo
end
```

In other circumstances, breaking out a method can avoid a type assertion (which
would itself involve at least one method call anyway).

For example, rather than:

```ruby
def hot_method(..)
  # ...
  x = T.let(polymorphic_factory(foo_please), Foo)
  # ...
end
```

Write:

```ruby
def hot_method(..)
  # ...
  x = make_foo
  # ...
end

sig {returns(FooType).checked(:tests)}
def make_foo
  polymorphic_factory(foo_please)
end
```

### Avoid constructing type objects

The construction of an non-trivial type object is typically the most expensive
part of a type assertion at runtime. One can usually mitigate this with the use
of `T.type_alias`.

For example, rather than:

```ruby
def hot_method(..)
  # ...
  foo = T.let({}, T::Hash[T.nilable(Symbol), T.any(Integer, Float)])
  # ...
end
```

Write:

```ruby
FooHash = T.type_alias { T::Hash[T.nilable(Symbol), T.any(Integer, Float)] }

def hot_method(..)
  # ...
  foo = T.let({}, FooHash)
  # ...
end
```

### Put type assertions behind memoization

Performance-sensitive methods are often memoized. In this case, it's usually
possible to memoize the runtime type check as well.

For example, rather than:

```ruby
sig {returns(Foo).checked(:tests)}
def foo
  @foo = T.let(@foo, T.nilable(Foo))
  @foo ||= something_expensive
end
```

Write:

```ruby
sig {returns(Foo).checked(:tests)}
def foo
  @foo ||= T.let(something_expensive, T.nilable(Foo))
end
```

Note that for class methods, there's a better option:

```ruby
@foo = T.let(nil, T.nilable(Foo))

sig {returns(Foo).checked(:tests)}
def self.foo
  @foo ||= something_expensive
end
```

### Inline type assertions using flow-sensitivity

A type assertion can usually be replaced by an explicit `===`, `is_a?` or
equivalent check of a local variable, which will avoid a method call. Sometimes
this makes code more verbose, but sometimes it can be a readability improvement
instead, especially in cases involving `T.must`.

For example, in place of:

```ruby
if foo.bar
  T.must(foo.bar).baz
end
```

Write:

```ruby
x = foo.bar
if x
  x.baz
end
```
