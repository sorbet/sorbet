---
id: rbs-support
title: RBS Comments Support
sidebar_label: RBS Comments
---

## Signature comments

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

Long signatures can be broken into multiple lines using the `#|` continuation
comment:

```ruby
#: (
#|  Integer,
#|  String
#| ) -> Float
def foo(x, y); end
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
- RBS supports certain enticing features, like duck typing.
- The syntax is terse.
- The Ruby distribution bundles a gem for parsing RBS annotations.

However, there are a number of problems which mean that RBS syntax has a
second-class position in Sorbet:

### RBS syntax does not match the semantics of Sorbet

Sorbet's type annotation syntax evolved differently from the evolution of RBS
syntax. Unfortunately, this difference is not syntax-deep: it affects the
semantics of the types too.

Sure, there are similarities: both syntaxes offer a way to express union types,
for example. But there are also semantic differences, and RBS syntax reflects
these differences. Things which are possible to express in RBS syntax have no
analogue in Sorbet and vice versa. Some examples:

- RBS supports duck typing via interfaces (different from Sorbet's
  [interfaces](abstract.md)), but
  [Sorbet does not support duck typing](faq.md#can-i-use-sorbet-for-duck-typed-code),
  by design.
- With Sorbet all class singleton classes are generic
  ([`T.class_of(...)[...]`](class-of.md#tclass_of-applying-type-arguments-to-a-singleton-class-type)).
  RBS does not have syntax to represent this.
- By extension, Sorbet allows singleton classes to declare their own generic
  type parameters (with
  [type_template](generics.md#type_member--type_template)). This also cannot be
  translated from RBS for the same limitation with RBS's singleton class type
  annotation.
- RBS supports literal value types. Sorbet does not.

Because of these differences, it's reasonable to assume that a codebase wishing
to take full advantage of Sorbet's unique features will eventually need to have
annotations that use `sig` syntax. The moment a method's annotation needs to use
Sorbet-only syntax, the entire annotation needs to get rewritten—it's not
possible to embed Sorbet-only syntax within the context of an RBS signature.
While it is possible for RBS comment signatures to coexist with Sorbet `sig`
signatures, needing to flip between them adds development friction.

### Sorbet has minimal influence over the evolution of RBS syntax

Sorbet continues to evolve its type syntax. For example, `T.anything`,
`T::Class`, and `has_attached_class!` are additions to Sorbet which arrived 6
years after Sorbet's inception—Sorbet development is anything but stale!

Sorbet can adapt more quickly to the needs of Sorbet users by building on its
own syntax. Historically, it has been difficult to advocate to adding RBS
features that benefit Sorbet specifically.

Our belief is that syntax matters less than semantics; that "what's possible to
express in the type system" matters more than "how to express it." Time spent
bikeshedding syntax detracts from meaningful improvements to the type system.

### IDE integration is more difficult

Sorbet's type syntax doubles as Ruby syntax. Setting aside other benefits of
reusing Ruby syntax for type annotations (e.g., [runtime checking](runtime.md),
no need to transpile, seamless integration with linters, etc.), Sorbet's syntax
integrates easily with existing editors.

There are a lot of features that come for free by reusing Ruby syntax:

- Syntax highlighting

  Highlighting for RBS comment-based type annotations is provided through the
  [Ruby LSP](https://github.com/Shopify/ruby-lsp). Without it, they will be
  monochromatic, highlighted like comments.

- Syntax error tolerance and recovery

  The RBS comment parser does not recover from syntax errors. If there's a
  syntax error in an RBS comment, Sorbet will ignore the entire `sig` until the
  error is fixed.

- Autocompletion

  Without error tolerance, autocompletion support is unreliable.

In fairness, these are technical, implementation considerations, and thus could
hope to improve one day. But they remain problems today.

### Performance is worse

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

#### Runtime checking is a feature

> Note that runtime checking of RBS signatures is not implemented, so type
> safety is reduced.

Sorbet's signatures are not just static type annotations: they are also
[checked at runtime](runtime.md). Runtime-checked signatures are a key reason
why Sorbet type annotations are so accurate: authors can't "lie" when writing
type annotations.

For more information on why runtime checking is valuable, see here:

→ [Runtime type checking is great](https://blog.jez.io/runtime-type-checking/)

In fact, runtime checking for signatures is actually load bearing: simply
disabling runtime checking can make the code change behavior when run.

#### The Sorbet docs will continue using Sorbet syntax

There are no plans to rewrite the Sorbet website to present RBS alternatives
alongside the existing Sorbet syntax. In the mean time, users will need to
mentally translate from Sorbet syntax to RBS syntax on their own, including
understanding the cases where there is no suitable RBS replacement.

## Quick reference

Most RBS features can be used and will be translated to equivalent Sorbet syntax
during type checking:

<table>
<thead><tr><th>RBS Feature</th><th>RBS syntax</th><th>Sorbet syntax</th></tr></thead>
<tbody>
<tr><td>

[Class instance type]

</td><td>

```plaintext
Foo
```

</td><td><a href="class-types">

```ruby
Foo
```

</a></td></tr>

<!-- end of Class instance type -->

<tr><td>

[Class singleton type]

</td><td>

```plaintext
singleton(Foo)
```

</td><td><a href="class-of">

```ruby
T.class_of(Foo)
```

</a></td></tr>

<!-- end of Class singleton type -->

<tr><td>

[Union type]

</td><td>

```plaintext
Foo | Bar
```

</td><td><a href="union-types">

```ruby
T.any(Foo, Bar)
```

</a></td></tr>

<!-- end of Union type -->

<tr><td>

[Intersection type]

</td><td>

```plaintext
Foo & Bar
```

</td><td><a href="intersection-types">

```ruby
T.all(Foo, Bar)
```

</a></td></tr>

<!-- end of Intersection type -->

<tr><td>

[Optional type]

</td><td>

```plaintext
Foo?
```

</td><td><a href="nilable-types">

```ruby
T.nilable(Foo)
```

</a></td></tr>

<!-- end of Optional type -->

<tr><td>

[Untyped type]

</td><td>

```plaintext
untyped
```

</td><td><a href="untyped">

```ruby
T.untyped
```

</a></td></tr>

<!-- end of Untyped type -->

<tr><td>

[Boolean type]

</td><td>

```plaintext
bool
```

</td><td><a href="class-types#booleans">

```ruby
T::Boolean
```

</a></td></tr>

<!-- end of Boolean type -->

<tr><td>

[Nil type]

</td><td>

```plaintext
nil
```

</td><td><a href="class-types#nil">

```ruby
NilClass
```

</a></td></tr>

<!-- end of Nil type -->

<tr><td>

[Top type]

</td><td>

```plaintext
top
```

</td><td><a href="anything">

```ruby
T.anything
```

</a></td></tr>

<!-- end of Top type -->

<tr><td>

[Bottom type]

</td><td>

```plaintext
bot
```

</td><td><a href="noreturn">

```ruby
T.noreturn
```

</a></td></tr>

<!-- end of Bottom type -->

<tr><td>

[Void type]

</td><td>

```plaintext
void
```

</td><td><a href="sigs#returns--void-annotating-return-types">

```ruby
void
```

</a></td></tr>

<!-- end of Void type -->

<tr><td>

[Generic type]

</td><td>

```plaintext
Foo[Bar]
```

</td><td><a href="generics">

```ruby
Foo[Bar]
```

</a></td></tr>

<!-- end of Generic type -->

<tr><td>

[Generic method]

</td><td>

```plaintext
[U] (U foo) -> U
```

</td><td><a href="generics#generic-methods">

```ruby
type_parameters(:U)
  .params(foo: T.type_parameter(:U))
  .returns(T.type_parameter(:U))
```

</a></td></tr>

<!-- end of Generic method -->

<tr><td>

[Tuple type]

</td><td>

```plaintext
[Foo, Bar]
```

</td><td><a href="tuples">

```ruby
[Foo, Bar]
```

</a></td></tr>

<!-- end of Tuple type -->

<tr><td>

[Shape type]

</td><td>

```plaintext
{ a: Foo, b: Bar }
```

</td><td><a href="shapes">

```ruby
{ a: Foo, b: Bar }
```

</a></td></tr>

<!-- end of Shape type -->

<tr><td>

[Proc type]

</td><td>

```plaintext
^(Foo) -> Bar
```

</td><td><a href="procs">

```ruby
T.proc.params(arg: Foo).returns(Bar)
```

</a></td></tr>

<!-- end of Proc type -->
</tbody>
</table>

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

Long attribute types can span over multiple lines:

```ruby
#: [
#|   Integer,
#|   String
#| ]
attr_reader :foo
```

## Method annotations

While RBS does not support the same modifiers as Sorbet, it is possible to
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

Note: these annotations like `@abstract` use normal comments, like `# @abstract`
(not the special `#:` comment). This makes it possible to reuse any existing
YARD or RDoc annotations.

## Class and module annotations

RBS annotations can be used to add Sorbet helpers to classes like
[`abstract!`](abstract.md):

```ruby
# @abstract
class Foo; end
end
```

This is equivalent to:

```ruby
class Foo
  extend T::Helpers

  abstract!
end
```

The [`@interface!`](abstract.md), [`@final!`](final.md), and
[`@sealed!`](sealed.md) annotations are supported in the same way.

The [`@requires_ancestor`](requires-ancestor.md) annotation expects an argument
to represent the ancestor to require:

```ruby
# @requires_ancestor: ::Some::Ancestor
class Foo; end
```

This is equivalent to:

```ruby
class Foo
  extend T::Helpers

  requires_ancestor { Some::Ancestor }
end
```

## Special behaviors

The `#:` comment must come **immediately** before the following method
definition. If there is a blank line between the comment and method definition,
the comment will be ignored.

Generic types like `Array` or `Hash` are translated to their `T::` Sorbet types
equivalent:

- `Array[Integer]` is translated to `T::Array[Integer]`
- `Class[Integer]` is translated to `T::Class[Integer]`
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

You can also consider using [`T::Enum`](tenum.md).

## Type assertions comments

> This feature is experimental and might be changed or removed without notice.
> To enable it pass the `--enable-experimental-rbs-assertions` option to Sorbet
> or add it to your `sorbet/config`.

### `T.let` assertions

[`T.let`](type-assertions.md#tlet) assertions can be expressed using RBS
comments:

```ruby
x = 42 #: Integer
@x = 42 #: Integer
X = 42 #: Integer
```

This is equivalent to:

```ruby
x = T.let(42, Integer)
@x = T.let(42, Integer)
X = T.let(42, Integer)
```

The comment must be placed at the end of the assignment. Either on the same line
if the assignment is on a single line, or on the last line if the assignment
spans multiple lines:

```ruby
x = [
  1, 2, 3
] #: Array[Integer]
```

The only exception is for HEREDOCs, where the comment may be placed on the first
line of the HEREDOC:

```ruby
X = <<~MSG #: String
  foo
MSG
```

### `T.cast` assertions

[`T.cast`](type-assertions.md#tcast) assertions can be expressed using RBS
comments with the `as` keyword:

```ruby
x = 42 #: as Integer
```

This is equivalent to:

```ruby
x = T.cast(42, Integer)
```

The comment is always applied to the outermost expression. For example, in this
method call, the cast is applied to the value returned by `foo` and not the
argument `x`:

```ruby
foo x #: as Integer
```

This is equivalent to:

```ruby
T.cast(foo x, Integer)
```

It is possible to cast the argument `x` by using parentheses:

```ruby
foo(
  x #: as Integer
)
```

This is equivalent to:

```ruby
foo(T.cast(x, Integer))
```

Casts comments can be used in any context where a type assertion is valid:

```ruby
foo
  .bar #: as Integer
  .baz

[
  foo, #: as Integer
  {
    bar: baz, #: as String
  }
]

x = if foo
  bar
else
  baz
end #: as Integer
```

### `T.must` assertions

[`T.must`](type-assertions.md#tmust) are denoted with the special `as !nil`
comment:

```ruby
foo #: as !nil
```

This is equivalent to:

```ruby
T.must(foo)
```

The `as !nil` comment can be used in any context where a cast is valid and
follows the same rules as `as Type` comments:

```ruby
foo(
  x #: as !nil
)
```

### `T.unsafe` escape hatch

[`T.unsafe`](static.md#call-site-granularity-tunsafe) can be replaced with the
special `as untyped` annotation:

```ruby
x = 42 #: as untyped
x.undefined_method # no error statically, but will fail at runtime
```

This is equivalent to:

```ruby
x = T.unsafe(42)
x.undefined_method
```

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
[Generic method]:
  https://github.com/ruby/rbs/blob/master/docs/syntax.md#type-variable
[Tuple type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#tuple-type
[Shape type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#record-type
[Proc type]: https://github.com/ruby/rbs/blob/master/docs/syntax.md#proc-type
