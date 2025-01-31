# Ruby Sorbet for VS Code

## Features

This extension provides language-aware IDE features for Ruby projects that use
Sorbet. It includes features like the following:

- Diagnostics (errors) that update as you type
- Hover tooltips, to show types and documentation
- Go to Definition/Find All References support
- Autocompletion
- Code Actions for refactoring
- Quick Fixes for errors

For a full list of features, see the "Editor Features" section of
[the Sorbet docs](https://sorbet.org/docs/vscode).

## Documentation

This extension only works in projects that have adopted Sorbet. For
instructions, see here:

- <https://sorbet.org/docs/adopting>

The docs for the Sorbet extension for VS Code live here:

- <https://sorbet.org/docs/vscode>

The Sorbet extension for VS Code is powered by the
[language server protocol](https://microsoft.github.io/language-server-protocol/)
(LSP). Sorbet's support for LSP is documented here:

- <https://sorbet.org/docs/lsp>

## Developing on this Extension

See [docs/lsp-dev-guide.md] for information on how to get started with LSP and
VS Code extension development.

[docs/lsp-dev-guide.md]: https://github.com/sorbet/sorbet/blob/master/docs/lsp-dev-guide.md
