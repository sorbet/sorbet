---
id: rbs-support
title: RBS comments support
sidebar_label: RBS comments support
---

> This feature is experimental and might be changed or removed without notice.
> To enable it pass the `--enable-experimental-rbs-signatures` option to Sorbet
> or add it to your `sorbet/config`.

Sorbet supports [RBS](https://github.com/ruby/rbs) syntax using comments in the
source code using the `#:` prefix:

```ruby
#: (Integer) -> String
def foo(x)
  T.reveal_type(x) # Revealed type: `Integer`

  x.to_s
end

str = foo(42)
T.reveal_type(str) # Revealed type: `String`
```

When Sorbet encounters a RBS comment during type checking, it will translate it
to the equivalent Sorbet syntax. Thus the previous example is strictly
equivalent to:

```ruby
sig { params(x: Integer).returns(String) }
def foo(x)
  ...
end
```

### Quick reference

Most RBS features can be used and will be translated to equivalent Sorbet syntax
during type checking:

| Feature              | RBS syntax                               | Sorbet syntax                          |
| -------------------- | ---------------------------------------- | -------------------------------------- |
| Class instance type  | `Foo`                                    | `Foo`                                  |
| Class singleton type | `singleton(Foo)`                         | `T.class_of(Foo)`                      |
| Union type           | <span><code>Foo &#124; Bar</code></span> | `T.any(Foo, Bar)`                      |
| Intersection type    | `Foo & Bar`                              | `T.all(Foo, Bar)`                      |
| Optional type        | `Foo?`                                   | `T.nilable(Foo)`                       |
| Boolean type         | `bool`                                   | `T::Boolean`                           |
| Nil type             | `nil`                                    | `NilClass`                             |
| Top type             | `top`                                    | `T.anything`                           |
| Bottom type          | `bot`                                    | `T.noreturn`                           |
| Void type            | `void`                                   | `void`                                 |
| Generic type         | `Foo[Bar]`                               | `Foo[Bar]`                             |
| Tuple type           | `[Foo, Bar]`                             | `[Foo, Bar]`                           |
| Shape type           | `{ a: Foo, b: Bar }`                     | `{ a: Foo, b: Bar }`                   |
| Proc type            | `^(Foo) -> Bar`                          | `T.proc.params(arg: Foo).returns(Bar)` |

### Attribute accessor types

Attribute accessors can be annotated with RBS types:

```ruby
#: String
attr_reader :foo

#: Integer
attr_writer :bar

#: String
attr_accessor :baz
```

#### Annotations

While RBS does not support the same modifiers than Sorbet, it is possible to
specify them using `@` annotation comments.

The following signatures are equivalent:

```ruby
# @abstract
#: (Integer) -> void
def foo1(x); end

sig { abstract.params(x: Integer).void }
def foo2(x); end

# @override
#: (Integer) -> void
def bar1(x); end

sig { override.params(x: Integer).void }
def bar2(x); end

# @override(allow_incompatible: true)
#: (Integer) -> void
def baz1(x); end

sig { override(allow_incompatible: true).params(x: Integer).void }
def baz2(x); end

# @final
#: (Integer) -> void
def qux1(x); end

sig(:final) { params(x: Integer).void }
def qux2(x); end
```

### Special behaviors

Generic types like `Array` or `Hash` are translated to their `T::` Sorbet types
equivalent:

- `Array[Integer]` is translated to `T::Array[Integer]`
- `Class[Integer]` is translated to `T.class_of(Integer)`
- `Enumerable[Integer]` is translated to `T::Enumerable[Integer]`
- `Enumerator[Integer]` is translated to `T::Enumerator[Integer]`
- `Enumerator::Lazy[Integer]` is translated to `T::Enumerator::Lazy[Integer]`
- `Enumerator::Chain[Integer]` is translated to `T::Enumerator::Chain[Integer]`
- `Hash[String, Integer]` is translated to `T::Hash[String, Integer]`
- `Range[Integer]` is translated to `T::Range[Integer]`
- `Set[Integer]` is translated to `T::Set[Integer]`

Note that non-generic types are not translated, so `Array` without a type
argument stays `Array`.

### Unsupported features

#### Class types

The `class` type in RBS is context sensitive and Sorbet does not support this
feature yet. Instead, use the equivalent Sorbet syntax:

```ruby
class Foo
  sig { returns(T.attached_class) }
  def self.foo; end
end
```

#### Interface types

Interface types are not supported, use the equivalent Sorbet syntax instead:

```ruby
module Foo
  extend T::Helpers

  interface!
end

#: (Foo) -> void
def takes_foo(x); end
```

#### Alias types

Alias types are not supported, use the equivalent Sorbet syntax instead:

```ruby
Bool = T.type_alias { T::Boolean }

sig { params(x: Bool).void }
def foo(x); end
```

#### Literal types

Literal types are not supported, use the equivalent class instance type instead:

- `1` is `Integer`
- `"foo"` is `String`
- `:foo` is `Symbol`
- `true` is `TrueClass`
- `false` is `FalseClass`
- `nil` is `NilClass`
