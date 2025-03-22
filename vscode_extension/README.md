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


## Sorbet Extension API

From 0.3.41, Sorbet exports a public API. Check VS Code's `getExtension` API. To ensure backward and forward compatibility,
all properties should be treated as are nullable.

 - `status()`: Sorbet status, or `undefined`.
 - `onStatusChanged`: event raised whenever status changes.

### Available Status Values
These are string values:
  - `disabled`: Sorbet Language Server has been disabled.
  - `error`:  Sorbet Language Server encountered an error. This state does not correlate to code typing errors.
  - `running`: Sorbet Language Server is running.
  - `start`: Sorbet Language Server is being started. The event might repeat in case of error.