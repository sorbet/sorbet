---
id: outline
title: Document Outline
sidebar_label: Outline & Document Symbols
---

Sorbet supports the `textDocument/documentSymbol` LSP feature, which powers many
editor features. In particular, this powers VS Code's [Outline], [Breadcrumbs],
and [Go to Symbol] features.

For example, in this screenshot we see VS Code's Outline view in the left
column, and the Breadcrumbs view across the top of the editor:

![](/img/lsp/outline-breadcrumbs.png)

Another example: this is what we see after pressing ⇧⌘O to bring up the Go to
Symbol interface, indicated by the `@` as the start of the line:

![](/img/lsp/go-to-symbol.png)

## VS Code Symbol Icons

The icons used to show document symbols are the same as the icons used in
completion results. Read [the VS Code docs][types of completions] to learn what
the icons mean, or see this chart:

![](/img/lsp/vscode-completion-list.png)

Other language clients may use different icons to represent the symbol kinds.

## Outlines in Test Files

As a side effect of how Sorbet models `describe`- and `it`-style tests, the
document outline can be used to navigate tests defined in a file:

![](/img/lsp/outline-breadcrumbs.png)

Notice how the `describe` blocks appear as if they were classes and the `it`
blocks appear as if they are methods. This can also be useful with Go to Symbol
to jump to a test defined in the current file.

[Outline]:
  https://code.visualstudio.com/docs/getstarted/userinterface#_outline-view
[Breadcrumbs]:
  https://code.visualstudio.com/docs/editor/editingevolved#_breadcrumbs
[Go to Symbol]:
  https://code.visualstudio.com/docs/editor/editingevolved#_go-to-symbol
[types of completions]:
  https://code.visualstudio.com/docs/editor/intellisense#_types-of-completions
