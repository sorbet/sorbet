---
id: doc-comments
title: Documentation Comments
---

Sorbet's language server has rudimentary support for displaying documentation
comments in various parts of the editor experience.

For example, hovering over things shows documentation associated with the
definition:

![](/img/hover-doc-comment.png)

Autocompletion results include documentation for the completion item:

![](/img/autocompletion-doc-comment.png)

## Adding documentation

There is no special syntax for defining documentation comments: Sorbet assumes
that any Ruby comment immediately before a definition is that definition's
documentation comment:

```ruby
# typed: true

# A simple class with some documentation
class A
  # The documentation for the A#foo method
  def foo; end
end

A.new.foo
```

To document a method with a signature, put the documentation comment above the
signature:

```ruby
# This is the documentation
sig { void }
def foo; end
```

This works for constants as well:

```ruby
# Documentation for X
X = 42
```

### Markdown support

Sorbet's language server will send documentation in Markdown format if the
language client declares that it supports Markdown (in the `contentFormat` of
`HoverClientCapabilities` and the `documentationFormat` of
`CompletionClientCapabilities` in the
[Language Server Protocol](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/)).
VS Code declares that it supports Markdown without configuration required from
the user, but other language clients may require special configuration.

![](/img/markdown-doc-comment.png)

Markdown support in documentation comments is especially useful for displaying
examples of usage in code blocks, like seen above.

### Documenting method parameters

At the moment, Sorbet does not support documenting individual method parameters.
Instead, we recommend documenting parameters in a method's documentation comment
with the `@param` annotation.

```ruby
  # @param name Who to greet
  sig { params(name: String).void }
  def self.greet(name)
    puts("Hello, #{name}!")
  end
```

Sorbet can generate a YARD snippet with `@param` and `@return` attributes for a
method, so you don't have to type them all manually:

<video autoplay loop muted playsinline style="max-width: calc(min(813px, 100%));">
  <source src="/img/lsp/yard-snippet.mp4" type="video/mp4">
</video>

Typing `##<TAB>` will accept a completion item from Sorbet which inserts a YARD
comment snippet.

See the [Autocompletion docs](autocompletion.md#completion-for-yard-snippets)
for more information.

## How Sorbet finds documentation comments

Sorbet tracks the locations of all definitions in a codebase.

To find a definition's documentation comment, it looks for a Ruby comment on the
line immediately before a definition. **Note**: a blank line between a comment
and a definition instructs Sorbet to **not** treat that comment as documentation
for the following definition.

```ruby
# This is NOT a documentation comment: it's just a normal comment in the file.
# It's not documentation because there's a blank line between this comment and
# the following definition.

def foo; end

# This comment IS a documentation comment, because there is no blank line.
def bar; end
```

If something is defined in multiple files, for example once in a Ruby source
file and once in an [RBI file](rbi.md), or a namespace which is reopened in
multiple files, Sorbet searches all of these known locations for a documentation
comment, and shows all that it finds.

```ruby
# -- file.rb --

# Documentation for A, comment 1
class A; end

# -- file.rbi --

# Documentation for A, comment 2
class A; end
```

**Note**: For performance, Sorbet only stores one location of a definition per
file. If something is defined multiple times within the same file, the last
location generally wins (and thus, only the last location of a definition within
a file will be searched for a documentation comment).
