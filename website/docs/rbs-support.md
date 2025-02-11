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

### Method signatures

Positional arguments names may or may not be specified:

```ruby
#: (Integer x) -> void
def foo(x); end

#: (Integer) -> void
def bar(x); end
```

For keyword arguments, the names must be specified:

```ruby
#: (x: Integer) -> void
def foo(x:); end
```

Optional positional and keyword arguments must be prefixed with `?`:

```ruby
#: (?Integer) -> void
def foo(x = 42); end

#: (Integer ?x) -> void
def bar(x = 42); end

#: (?x: Integer) -> void
def baz(x: 42); end
```

#### Block parameters

Block parameters can be specified using the `{}` syntax:

```ruby
#: (Integer) { (Integer) -> String } -> void
def foo(x, &block); end
```

Optional block parameters must be prefixed with `?`:

```ruby
#: (Integer) ?{ (Integer) -> String } -> void
def bar(x, &block); end
```

Self binding for blocks can be specified using the `[self]` syntax:

```ruby
#: (Integer) { (Integer) [self: Foo] -> String } -> void
def baz(x, &block); end
```

#### Proc parameters

Proc parameters can be specified using the `^` syntax:

```ruby
#: (Integer, ^(Integer) -> String) -> void
def baz(x, proc); end
```

#### Generic methods

For generic methods, the type parameters must be specified using the `[]`
syntax:

```ruby
#: [X] (X) -> Class[X]
def foo(x); end
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

### Special behaviors

Generic `Array`, `Hash`, `Set`, and `Class` types are translated to their `T::`
Sorbet types equivalent:

- `Array[Integer]` is translated to `T::Array[Integer]`
- `Hash[String, Integer]` is translated to `T::Hash[String, Integer]`
- `Set[Integer]` is translated to `T::Set[Integer]`
- `Class[Integer]` is translated to `T.class_of(Integer)`

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
