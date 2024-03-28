---
id: go-to-def
title: Go to Definition, Type Definition, and Implementations
sidebar_label: Go to Definition
---

Sorbet supports various "Go to Definition" features via LSP.

<video autoplay loop muted playsinline style="max-width: calc(min(314px, 100%)); display:block;margin-left:auto;margin-right:auto;">
  <source src="/img/lsp/go_to_def.mp4" type="video/mp4">
</video>

As with most LSP features, support for Go to Definition works best in
`# typed: true` files. For more, see
[Feature support by strictness level](lsp-typed-level.md).

## Troubleshooting

### No definition results

Double check [Feature support by strictness level](lsp-typed-level.md).

If the file is at least `# typed: true` and there are still no results, double
check whether the value is [`T.untyped`](untyped.md). Check this either by
hovering over the expression, or by wrapping the expression in
[`T.revel_type`](troubleshooting.md#treveal_type).

### Wrong definition location

Please try to reproduce the issue in the [Sorbet Playground](https://sorbet.run)
and report an issue (click the "Create issue with example" button from the
"Examples" dropdown menu).

### Jumping to the definition took me to a webpage, instead of a Ruby file

The [RBI files](rbi.md) that Sorbet uses to define everything from the Ruby
standard library only exist as binary data in Sorbet's executable, not as actual
files on disk.

To support jumping into these files, Sorbet has support for
[Working with Synthetic or Missing Files](sorbet-uris.md). This support is built
into the [Sorbet VS Code extension](vscode.md), but may require extra
integration work in other language clients.

When using this feature, Sorbet returns special `sorbet:` URIs when attempting
to jump into synthetic or missing files, which allows them to be opened as
read-only files, directly in the editor.

When not using this feature, Sorbet responds to "Go to Definition" requests on
these definitions by returning an `https://` URI to the Sorbet repo on GitHub,
where those RBI files live. Most language clients will interpret this `https://`
URI by opening the default web browser. Some language clients may attempt to
fetch the raw HTML of the page, instead of opening the URI in a browser.

See [Working with Synthetic or Missing Files](sorbet-uris.md) for more
information.

## Definition vs Type Definition

The "Go to Definition" feature goes to wherever the thing under the cursor was
defined. For example, if the thing under the cursor is a local variable, it goes
where that local variable was first assigned. If it's a method call, it goes to
the definition of that method.

By contrast, the "Go to Type Definition" feature first determines the type of
the expression under the cursor. Regardless of whether that's a method call or a
local variable or something else, it determines the type of the expression. Then
Sorbet figures out where that _type_ was defined. For simple
[Class Types](class-types.md) like `MyClass` or `Integer`, Sorbet jumps to the
definition of the class. For complex types like [Union Types](union-types.md),
Sorbet returns the locations of every component in the union. The language
client usually presents these options and asks the user to pick which definition
to go to.

## Go to Implementations / Find All Implementations

With the cursor on an abstract method's name (either a call to the method, or
its definition), Sorbet can list all the implementations of that abstract
method.

This feature also works to find subclasses of an abstract class.

<video autoplay loop muted playsinline style="max-width: calc(min(813px, 100%));">
  <source src="/img/lsp/find-all-implementations.mp4" type="video/mp4">
</video>

## Go to Overridden Method

The `override` keyword in a method signature is a Go to Definition target:

```ruby
class Parent
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.void } # ◀── jumps to here
  def foo; end
end

class Child < Parent
  sig { override.void }
  #       ▲
  #       └─── Go to Definition here
  def foo; end
end
```

Using "Go to Definition" on the `override` modifier in a signature jumps to the
method that it overrides, usually a corresponding [`abstract`](abstract.md) or
[`overridable`](override-checking.md) method.

## Go to Definition on `super`

The `super` keyword is a Go to Definition target. Clicking it jumps to the
corresponding super method, i.e. the first method in the ancestor chain of the
current class with the same name as the current method.

```ruby
class Parent
  def initialize # ◀── jumps to here
  end
end

class Child < Parent
  def initialize
    super
    # ▲
    # └─── Go to Definition here
  end
end
```
