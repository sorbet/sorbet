# test/sandbox/vscode

```
code --new-window .
```

You will have to use the `> Sorbet: Configure` command to select the "Sorbet
(path)" configuration, which will use the Sorbet built into
`../../../bazel-bin/main/sorbet` (whatever was most recently built).

NOTE: launching VS Code this way will default to using the Sorbet VS Code
extension from the marketplace. See `vscode_extension/README.md` folder for
instructions on how to test local changes to the VS Code extension.

This config is meant just as a quick-and-dirty way to test changes to the Sorbet
language server if no client changes are needed.
