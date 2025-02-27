---
id: rbs-support
title: RBS comments support
sidebar_label: RBS comments support
---

> This feature is experimental and might be changed or removed without notice.
> To enable it pass the `--enable-experimental-rbs-signatures` option to Sorbet
> or add it to your `sorbet/config`.

Sorbet has experimental support for comment-only type syntax, powered by
[RBS](https://github.com/ruby/rbs) annotations.

The syntax looks like this:

```ruby
#: (Integer) -> String
def foo(x)
  T.reveal_type(x) # Revealed type: `Integer`

  x.to_s
end

str = foo(42)
T.reveal_type(str) # Revealed type: `String`
```

This feature is powered by translating RBS comments to equivalent `sig` syntax.
For example, the previous example is similar to having written a `sig` directly:

```ruby
sig { params(x: Integer).returns(String) }
def foo(x)
  ...
end
```

## Caveats

> Support for this feature is experimental, and we actively discourage depending
> on it for anything other than to offer feedback to the Sorbet developers.

There are numerous shortcomings of the comment-based syntax versus `sig` syntax.
By contrast, the near-sole upside is that the syntax is terser.

### The comment-based syntax is second class

The comment-based syntax uses [RBS syntax](https://github.com/ruby/rbs). RBS is
an alternative annotation syntax for Ruby. The headline features of RBS:

- It's the type annotation syntax blessed by the Ruby core team, and Matz in
  particular (the lead of the Ruby project).
- RBS supports certain enticing features, like method overloading and duck
  types.
- The syntax is terse.
- The Ruby distribution bundles a gem for parsing RBS annotations.

However, there are a number of problems which mean that RBS syntax has a
second-class position in Sorbet:

#### RBS syntax does not match the semantics of Sorbet

Sorbet's type annotation syntax evolved differently from the evolution of RBS
syntax. Unfortunately, this difference is not syntax-deep: it affects the
semantics of the types too.

Sure, there are similarities: both syntaxes offer a way to express union types,
for example. But there are also semantic differences, and RBS syntax reflects
these differences. Things which are possible to express in RBS syntax have no
analogue in Sorbet and vice versa. Some examples:

- RBS supports duck typing, but
  [Sorbet does not](faq.md#can-i-use-sorbet-for-duck-typed-code), by design
- Sorbet allows singleton classes to be generic (e.g.
  [type_template](generics.md#type_member--type_template)). RBS has no such
  concept.
- Sorbet treats Ruby's `::Class` as a generic class, allowing sophisticated
  class-level metaprogramming. RBS does not have syntax to represent this.

Because of these differences, it's reasonable to assume that a codebase wishing
to take full advantage of Sorbet's unique features will eventually need to have
annotations that use `sig` syntax. The moment a method's annotation needs to use
Sorbet-only syntax, the entire annotation needs to get rewritten, adding
friction to the development experience.

#### Sorbet has minimal influence over the evolution of RBS syntax

Sorbet continues to evolve its type syntax. For example, `T.anything`,
`T::Class`, and `has_attached_class!` are additions to Sorbet which arrived 6
years after Sorbet's inception—Sorbet development is anything but stale!

Sorbet can adapt more quickly to the needs of Sorbet users by building on its
own syntax. Historically, it has been difficult to advocate to adding RBS
features that benefit Sorbet specifically.

Our belief is that syntax matters less than semantics; that "what's possible to
express in the type system" matters more than "how to express it." Time spent
bikeshedding syntax detracts from meaningful improvements to the type system.

#### IDE integration is more difficult

Sorbet's type syntax doubles as Ruby syntax. Setting aside other benefits of
reusing Ruby syntax for type annotations (e.g., [runtime checking](runtime.md),
no need to transpile, seamless integration with linters, etc.), Sorbet's syntax
integrates easily with existing editors.

There are a lot of features that come for free by reusing Ruby syntax:

- Syntax highlighting

  There are no plans to offer highlighting for RBS comment-based type
  annotations. They will be monochromatic, highlighted like comments
  (third-party extensions may offer such functionality).

- Syntax error tolerance and recovery

  The RBS comment parser does not recover from syntax errors. If there's a
  syntax error in an RBS comment, Sorbet will ignore the entire `sig` until the
  error is fixed.

- Autocompletion

  Without error tolerance, autocompletion support is unreliable.

In fairness, these are technical, implementation considerations, and thus could
hope to improve one day. But they remain problems today.

#### No support for inline type annotations

At time of writing, there is no support for inline type annotations and
assertions (e.g., no `T.let` nor `T.cast` alternative).

Even if they do eventually come, they would only apply to a variable declaration
on its own line. Because Ruby does not have something akin to C-style
`/* ... */` comments, any comment-based replacement for `T.let` would only
support trailing comments, meaning only for variable assignments. Meanwhile,
`T.let` can be used anywhere inside an expression.

#### Performance is worse

The chosen implementation for RBS annotations in comments is:

1.  Scan and parse the file, like normal.
2.  Using the parsed file contents, re-scan the file, looking for comments,
    because Sorbet does not feed parsed comments throughout its pipeline.
3.  Parse the RBS comments, using a third-party RBS parser, which manages memory
    allocations using it's own internal data structures.
4.  Translate the parsed RBS types to equivalent Ruby ASTs, and splice those
    ASTs into the parse result.
5.  Allow Sorbet to continue, where signatures and type annotations will be
    analyzed later in the pipeline.

```
┌──────────────┐
│  scan+parse  │
└──────┬───────┘
       ├──────────────────────────┐
       │                          ▼
       │                   ┌──────────────┐
       │                   │  scan again  │
       │                   └──────┬───────┘
       │                          ▼
       │                   ┌─────────────┐
       │                   │  parse RBS  │
       │                   └──────┬──────┘
       │                          ▼
       │                   ┌─────────────┐
       │                   │  translate  │
       │                   └─────────────┘
       ├──────────────────────────┘
       ▼
┌────────────────┐
│  ingest types  │
└────────────────┘
```

This implementation is simple, making it easy to verify correctness. The RBS
parser can be developed as a library and tested independently. As long as the
translation produces Sorbet ASTs equivalent to `sig` annotations that a user
would have written, ingesting the types will work correctly.

But steps 2 and 4 are pure overhead (step 3 is a wash). Not only does this
implementation scan every file which _might_ use RBS comments twice, but it does
not parse straight into Sorbet's internal type representation.

In large codebases, this adds nontrivial overhead, and is a blocker in the way
of being able to advocate for using this syntax more widely.

### Runtime checking is a feature

Sorbet's signatures are not just static type annotations: they are also
[checked at runtime](runtime.md). Runtime-checked signatures are a key reason
why Sorbet type annotations are so accurate: authors can't "lie" when writing
type annotations.

For more information on why runtime checking is valuable, see here:

→ [Runtime type checking is great](https://blog.jez.io/runtime-type-checking/)

In fact, runtime checking for signatures is actually load bearing: simply
disabling runtime checking can make the code change behavior when run.

### The Sorbet docs will continue using Sorbet syntax

There are no plans to rewrite the Sorbet website to present RBS alternatives
alongside the existing Sorbet syntax. In the mean time, users will need to
mentally translate from Sorbet syntax to RBS syntax on their own, including
understanding the cases where there is no suitable RBS replacement.

## Quick reference

Most RBS features can be used and will be translated to equivalent Sorbet syntax
during type checking:

| RBS Feature            | RBS syntax                               | Sorbet syntax                                           |
| ---------------------- | ---------------------------------------- | ------------------------------------------------------- |
| [Class instance type]  | `Foo`                                    | [`Foo`](class-types.md)                                 |
| [Class singleton type] | `singleton(Foo)`                         | [`T.class_of(Foo)`](class-of.md)                        |
| [Union type]           | <span><code>Foo &#124; Bar</code></span> | [`T.any(Foo, Bar)`](union-types.md)                     |
| [Intersection type]    | `Foo & Bar`                              | [`T.all(Foo, Bar)`](intersection-types.md)              |
| [Optional type]        | `Foo?`                                   | [`T.nilable(Foo)`](nilable-types.md)                    |
| [Untyped type]         | `untyped`                                | [`T.untyped`](untyped.md)                               |
| [Boolean type]         | `bool`                                   | [`T::Boolean`](class-types.md#booleans)                 |
| [Nil type]             | `nil`                                    | [`NilClass`](class-types.md#nil)                        |
| [Top type]             | `top`                                    | [`T.anything`](anything.md)                             |
| [Bottom type]          | `bot`                                    | [`T.noreturn`](noreturn.md)                             |
| [Void type]            | `void`                                   | [`void`](sigs.md#returns--void-annotating-return-types) |
| [Generic type]         | `Foo[Bar]`                               | [`Foo[Bar]`](generics.md)                               |
| [Tuple type]           | `[Foo, Bar]`                             | [`[Foo, Bar]`](tuples.md)                               |
| [Shape type]           | `{ a: Foo, b: Bar }`                     | [`{ a: Foo, b: Bar }`](shapes.md)                       |
| [Proc type]            | `^(Foo) -> Bar`                          | [`T.proc.params(arg: Foo).returns(Bar)`](procs.md)      |

## Attribute accessor types

Attribute accessors can be annotated with RBS types:

```ruby
#: String
attr_reader :foo

#: Integer
attr_writer :bar

#: String
attr_accessor :baz
```

### Annotations

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

## Special behaviors

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

## Unsupported features

### Class types

The `class` type in RBS is context sensitive (depends on the class where it is
used) and Sorbet does not support this feature yet. Instead, use the equivalent
Sorbet syntax:

```ruby
class Foo
  sig { returns(T.attached_class) }
  def self.foo; end
end
```

### Interface types

Interface types are not supported, use the equivalent Sorbet syntax instead:

```ruby
module Foo
  extend T::Helpers

  interface!
end

#: (Foo) -> void
def takes_foo(x); end
```

### Alias types

Alias types are not supported, use the equivalent Sorbet syntax instead:

```ruby
Bool = T.type_alias { T::Boolean }

sig { params(x: Bool).void }
def foo(x); end
```

### Literal types

Sorbet does not support RBS's concept of "literal types". The next best thing is
to use the literal's underlying type instead:

- `1` is `Integer`
- `"foo"` is `String`
- `:foo` is `Symbol`
- `true` is `TrueClass`
- `false` is `FalseClass`
- `nil` is `NilClass`

You can also consider using [`T.::Enum`](tenum.md).

[Class instance type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#class-instance-type
[Class singleton type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#class-singleton-type
[Union type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#union-type
[Intersection type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#intersection-type
[Optional type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#optional-type
[Untyped type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#base-types
[Boolean type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#bool-or-boolish
[Nil type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#nil-or-nilclass
[Top type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#base-types
[Bottom type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#base-types
[Void type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#base-types
[Generic type]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#type-variable
[Tuple type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#tuple-type
[Shape type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#record-type
[Proc type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#proc-type
