---
id: autocompletion
title: Autocompletion
sidebar_label: Autocompletion
---

Sorbet supports autocompletion via LSP.

<video autoplay loop muted playsinline style="max-width: calc(min(934px, 100%));">
  <source src="/img/lsp/autocompletion.mp4" type="video/mp4">
</video>

Some basic features of autocompletion in Sorbet:

- Sorbet supports completion for method calls, local, instance, and class
  variables, Ruby keywords, classes, and constants.

- Completion items will include documentation for the suggestion, when
  available. Sorbet assumes that a comment immediately above the definition of
  things like classes, methods, and constants is the documentation for a given
  item.

  Different language clients may or may not show this information by default.

- Completion items are sorted by inheritance. Methods defined lower in the
  inheritance hierarchy show up higher in the completion results. (This is not a
  perfect heuristic, but it is how Sorbet works.)

There are some more complicated features worth calling out in their own section.

## Troubleshooting

### No completion results

Support for autocompletion is minimal in `# typed: false` files,
[like most other LSP features](lsp-typed-level.md#support-by-lsp-feature).

If the file is `# typed: true` and there are still no completion results, it
might be that Sorbet is busy with an ongoing operation. See
[Showing the Language Server Status](server-status.md) to understand what the
server statuses mean, and whether Sorbet is able to respond to completion
requests given a certain status.

If Sorbet is "Idle" (or "Typechecking in background...") and there are still no
completion results, it might be that Sorbet failed to recover from a syntax
error in the file. See
[Syntax error recovery and completion](#syntax-error-recovery-and-completion).

If there is no syntax error, it might be that the code path is unreachable.
Sorbet does not show completion results for dead/unreachable code paths.

### Wrong completion results

If the completion results are present but look wrong, inspect the kind of
completion item. For example, VS Code uses these icons to show the completion
item kinds:

![](/img/lsp/vscode-completion-list.png)

Sorbet will only ever return completion items with the kind `method`,
`variable`, `field`, `class`, `interface`, `module`, `enum`, or `keyword` (and
sometimes `snippet`).

**Notably**, the "abc" icon (`word`) means the results came either from VS
Code’s `editor.wordBasedSuggestions` setting or some other generic autocomplete
extension. (Or, if not using VS Code, then from some other plugin.) Sorbet
**never** produces `word` completion items.

In specific circumstances (see
[Completion for method signatures](#completion-for-method-signatures) and
[Completion for YARD snippets](#completion-for-yard-snippets)), Sorbet will also
produce `snippet` items. In all other cases, `snippet` items come from some
other snippet provider, not Sorbet.

If the completion results still look wrong, please
[report a bug](https://github.com/sorbet/sorbet/issues/new/choose).

## Syntax error recovery and completion

One of the biggest reasons why Sorbet fails to produce completion results is
because it failed to recover from a syntax error in the file.

Sorbet has a custom Ruby syntax parser which attempts to recover from many
common syntax errors, but it is not perfect. For example:

```ruby
def foo(x)
  x.
end
```

This Ruby program has a syntax error, and yet Sorbet is able to recover from it,
understanding that this method contains a call on `x` which is missing the
method name.

To see all the known cases where Sorbet fails to recover from parsing a file
with a syntax error, see
[all issues labeled `parser`](https://github.com/sorbet/sorbet/issues?q=is%3Aissue+is%3Aopen+label%3Aparser).

The biggest known case where Sorbet fails to recover is with mismatched
parentheses, brackets, and quotes.

If you think you've found another example that Sorbet should be able to recover
from, please open an issue. The [Sorbet Playground](https://sorbet.run) allows
showing the parse result for a file, which is useful for debugging whether
Sorbet parsed or failed to parse a given syntax error: simply pass [the
`?arg=--print=parse-tree-whitequark` flag][bad-parse] in the query string.

[bad-parse]:
  https://sorbet.run/?arg=--print=parse-tree-whitequark#%23%20typed%3A%20true%0A%23%20Craft%20your%20test%20case%2C%20then%20click%20%22Create%20issue%20with%20example%22%0A%23%20in%20the%20%22Examples%20%E2%98%B0%22%20menu%20above.%0Adef%20foo%0A%20%20puts%20'hello'%0Aend

## Completion for method signatures

Sorbet can suggest signatures for methods without signatures. Simply typing
`sig` above a method without a signature will trigger a completion item that
expands to a snippet with one parameter in the `sig` for each parameter in the
method definition immediately following the `sig`.

For more information, see
[Automatically suggesting method signatures](sig-suggestion.md)

![](/img/suggest-sig-completion-item-01.png)

## Pre-declared snippets

Snippets expand a pre-defined template and allow pieces of the template to be
filled in with values (most language clients allow pressing `TAB` to cycle
through the parts of the template that need to be filled in).

Sorbet includes some snippet autocompletion results. These show up in addition
to any snippets that editors might already be configured with. Snippets from
Sorbet always show up prefixed with `(sorbet)`:

![](/img/lsp/struct-snippet.png)

In VS Code, snippets also show up with a "square with dashed bottom line" icon.
See [Wrong completion results](#wrong-completion-results).

If a snippet has the "square with dashed bottom line" icon but does not start
with `(sorbet)`, it came from some other extension in your developer environment
and is configured separately.

These are the snippets Sorbet includes.

### Snippets for all Ruby keywords.

For example, typing `case` brings up a completion item which, when accepted,
expands to

```ruby
case expr
when expr

else
end
```

Other notable keyword snippets include snippets for `def`, `class`, `module`,
and `if`.

### Snippets for Sorbet-specific constructs.

You can type `struct` or `enum` and have these auto-expand to
[T::Struct](tstruct.md) and [T::Enum](tenum.md) classes:

<video autoplay loop muted playsinline style="max-width: calc(min(727px, 100%));">
  <source src="/img/lsp/struct-enum-snippet.mp4" type="video/mp4">
</video>

The `struct` and `enum` snippet triggers are not methods that exist at runtime:
they're only indicators to Sorbet that you're trying to define a `T::Struct` or
`T::Enum`.

These completions will only show up if there is no explicit receiver for a
method call (e.g., `x.struct` will not show the `T::Struct` snippet suggestion,
only `struct`).

### Completion for YARD snippets

Sorbet can suggest a YARD doc snippet for methods.

<video autoplay loop muted playsinline style="max-width: calc(min(813px, 100%));">
  <source src="/img/lsp/yard-snippet.mp4" type="video/mp4">
</video>

It will appear after typing `##` above a Ruby method or signature, and
pre-populate the snippet with one `@param` for each parameter the method has.
Some reminders about these docs:

- Sorbet recognizes Markdown syntax in these docs, and VS Code will render the
  Markdown to rich text when showing the documentation.

  Get creative and show examples of how to use your API!

- These docs show up on hover and in autocompletion items.

- Sorbet **does not** read YARD docs for type annotations.

- See [the YARD docs][available-yard-tags] for a list of available tags.

[available-yard-tags]:
  https://rubydoc.info/gems/yard/file/docs/Tags.md#List_of_Available_Tags
