---
id: attr_reader
title: attr_reader, attr_writer, and attr_accessor
sidebar_label: attr_reader
---

Sorbet has special support to allow defining signatures for Ruby's `attr_reader`, `attr_writer`, and `attr_accessor` methods.

```ruby
sig { returns(Integer) }
attr_reader :foo

sig { params(bar: Integer).returns(Integer) }
attr_writer :bar

sig { returns(Integer) }
attr_accessor :qux
```

For `attr_accessor`, simply specify the return type. Sorbet will implicitly use the return type as the parameter type for the setter method defined by `attr_accessor`.

For `attr_writer`, the parameter name in the signature must be the same as the name of the attribute.

## Multiple attributes

```ruby
sig { returns(Integer) }
attr_accessor :x, :y
```

Ruby allows the `attr_*` methods to accept multiple symbols, defining multiple attribute methods.

Sorbet only allows using this form when all the attributes have the same type. In this form, one declared signature will be implicitly reused for all listed attribute names.

Otherwise, Sorbet requires that the attribute list is broken up to give unique types to each attribute:

```ruby
sig { returns(Integer) }
attr_accessor :x
sig { returns(String) }
attr_accessor :y
```

## In `# typed: strict` files

`attr_*`-defined methods are backed by instance variables. Sorbet requires types for instance variables in `# typed: strict` files.

To define types for these instance variables, either:

1.  Ensure that the type for the attribute is [T.nilable(...)](nilable-types.md) **and** that there is a writer defined (via `attr_writer` or `attr_accessor`), or

    ```ruby
    sig { returns(T.nilable(Integer)) }
    attr_accessor :foo
    ```

1.  Manually define the type for the instance variable, like [any other instance variable](type-annotations.md#declaring-class-and-instance-variables).

    ```ruby
    sig { returns(Integer) }
    attr_accessor :foo

    sig { void }
    def initialize
      @foo = T.let(0, Integer)
    end
    ```

Note that due to limitations in Sorbet, the `T.nilable` **must be visible syntactically**. Hiding the `T.nilable` in a [type alias](type-aliases.md) or some other complicated type will not work:

```ruby
NilableInteger = T.type_alias { T.nilable(Integer) }

sig { returns(NilableInteger) }
attr_accessor :foo

sig { void }
def example
  p(@foo) # 💥 error: Use of undeclared variable `@foo`
end
```

It's unlikely that this restriction will be lifted in the future.

## Runtime type checking

Adding a signature to an attribute **only** declares a runtime signature for the **first** method defined from that call to the `attr_*` method. Any method defined after will not have a runtime signature. For example:

```ruby
# only defines a runtime signature for the getter method `foo()`
# DOES NOT define a runtime signature for the setter method `foo=(foo)`
sig { returns(Integer) }
attr_accessor :foo

# only defines a runtime signature for the first getter method `foo()`
# DOES NOT define a runtime signature for second/third getter methods (`bar()`, `qux()`)
sig { returns(Integer) }
attr_reader :foo, :bar, :qux
```

This is due to a technical limitation in `sorbet-runtime`, but which can hopefully be lifted in the future. See [this issue](https://github.com/sorbet/sorbet/issues/5685) for prior art.

## In the IDE

Methods defined with `attr_reader`, `attr_writer`, or `attr_accessor` are shown specially in IDEs, like the [document outline](outline.md) and the [completion item list](autocompletion.md). They're reported as the `property` symbol kind, which [in VS Code](outline.md#vs-code-symbol-icons) is shown with a 🔧 icon.

## Attributes and flow sensitivity

Sorbet does not track flow sensitivity for attribute-defined methods.

In Ruby, things defined by `attr_reader`, `attr_writer`, and `attr_accessor` are not "special": these DSLs just define normal methods. It just so happens that Ruby allows calling methods without trailing parentheses (e.g. `foo()`), so it sometimes looks like attribute-defined methods are special.

Sorbet can only track control flow-sensitive types on variables, not method calls. This is the exact same limitation that other popular gradual type checkers except for one difference: both JavaScript and Python make a syntactic distinction between method calls (which have parentheses) versus property/attribute access (which don't).

For more on how this affects Ruby, see this post:

→ [Control Flow in Sorbet is Syntactic](https://blog.jez.io/syntactic-control-flow/#properties-and-attributes-in-other-languages)
