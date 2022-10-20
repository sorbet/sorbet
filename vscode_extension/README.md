# Ruby Sorbet for VS Code

## Features:

Provides Ruby editor features using the Sorbet language server:

- Live typechecking
- Types and documentation on hover
- Go-to-definition
- Find all references in workspace
- Workspace symbol search
- Autocompletion of methods, local variables, keywords, and `sig`s.

## Getting started

By default, this extension attempts to use a Sorbet language server
that it launches using `bundle exec srb typecheck --lsp`.

In a Ruby environment that includes the `sorbet` gem, where `srb init` has been
run as per [Sorbet's "Getting Started" documentation](https://sorbet.org/docs/adopting),
this is often sufficient to get started with the Sorbet language server in
_typed files_ (i.e., those marked as `#typed: true` or stricter).

Use `sorbet.lspConfigs` to customize how the Sorbet language server is launched.
If VSCode settings are checked into source control, users may wish to set
`sorbet.userLspConfigs` globally to avoid dirtying/modifying their local
`.vscode/settings.json`. (Configs from `sorbet.userLspConfigs` override those from
`sorbet.lspConfigs` with matching ids.)

Read the [documentation on the Sorbet website](https://sorbet.org/docs/vscode)
for further information.

## Caveats/Disclaimers

- The Sorbet language server does not provide language support for symbols
  in untyped files (i.e., those ignored or effectively marked `# typed: false`).
  See [the Sorbet CLI docs](https://sorbet.org/docs/cli) for more information
  about how to change the scope of files considered by Sorbet scope.

- Sorbet's language server supports flags to enable beta-testing
  and experimental features; users should be aware that these features
  are constantly in flux, may occasionally be wildly incompatible with
  this extension, and are likely to change without notice.

## Developing on this Extension

```
cd vscode_extension
code --new-window .
```

To test out the changes:

- Find the "Run and Debug" window
- Ensure that "Launch Extension" is selected in the dropdown
- Click the green triangle, which will launch a new VS Code instance with the
  local copy of the extension loaded
- Navigate to the project you want to test Sorbet in

Or in one command: ⇧⌘P > `Debug: Select and Start Debugging` > `Launch Extension`
