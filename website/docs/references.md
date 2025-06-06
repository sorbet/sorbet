---
id: references
title: Find All References
---

Sorbet can show the types and documentation for parts of the program via the LSP Hover feature.

<img src="/img/lsp/references.png" style="max-width: 499px"/>

[View on sorbet.run →](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Parent%0A%20%20def%20foo%3B%20end%0Aend%0A%0Aclass%20NotParent%0A%20%20def%20foo%3B%20end%0Aend%0A%0Aclass%20Child1%20%3C%20Parent%3B%20end%0Aclass%20Child2%20%3C%20Parent%3B%20end%0A%0AParent.new.foo%0A%23%20%20%20%20%20%20%20%20%20%20%5E%5E%5E%20right%20click%20and%20%22Find%20All%20References%22%20here)

<!-- TODO(jez) Eventually, we should have a section here about how find all references treats package files specially -->

## Troubleshooting

### Find All References keeps spinning

Sorbet only begins to service "Find All References" requests when in the Idle state. (Sorbet does not run Find All References in the background. This is for simplicity in Sorbet's implementation, and could be relaxed in the future.)

Read about all of Sorbet's [server statuses](server-status.md) for more information on these states.

### Some results are missing

Sorbet will only be able to find references on typed usage sites. In particular for methods calls, the file must be at least `# typed: true` and the method receiver must be typed.

See [Feature support by strictness level](lsp-typed-level.md#support-by-lsp-feature) for more information on how Find All References interacts with the `# typed:` level.

Note that Sorbet should never fail to find references to a constant (except those in `# typed: ignore` files). Report a bug if this is not the case.

### Find All References is slow

The speed of Find All References depends on how many files contain an identifier with the same name, as we use that information as a first-pass filter over the source files before refining to actual references. For example, searching for references to a class's `initialize` will involve a scan over most files in a project even if the specific `initialize` you are looking for is only used in one file.

### Find All References brought me to a file that I cannot edit

These features may return results in type definitions for core Ruby libraries, which are baked directly into the Sorbet executable and are not present on the file system.

In order to display these files in your editor and to support navigating through them, the Sorbet extension displays them in a read-only view. Note that certain extension features, like Hover and Go to Definition, will not function in some of these special files.

See [Working with Synthetic or Missing Files](sorbet-uris.md) for more on this behavior.

### I only want to see certain references

Neither the [LSP](lsp.md) specification nor VS Code provide the ability to filter the list of reference results. (See for example [this issue](https://github.com/microsoft/vscode/issues/205534) in the VS Code issue tracker).

Other language clients (like Vim or Neovim) have better support for filtering the list of references, and there may be other VS Code extensions which provide third-party ways of filtering the list of references.
