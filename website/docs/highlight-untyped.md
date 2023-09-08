---
id: highlight-untyped
title: Highlighting untyped code
sidebar_label: Highlighting untyped
---

> **Note**: This feature is in beta. Please give us feedback!

Sorbet can highlight regions of [untyped] Ruby code in editors. Here's what it
looks like:[^1]

[untyped]: untyped.md

![](/img/highlight-untyped.png)

[^1]:
    This screenshot uses the [Error Lens] VS Code extension to display
    diagnostic titles inline.

[error lens]:
  https://marketplace.visualstudio.com/items?itemName=usernamehw.errorlens

VS Code renders these untyped code highlights with a blue squiggly underline.
Other language clients may present them differently, depending on how they
render diagnostics with an [Information] severity.

[information]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticSeverity

## Toggling untyped highlights

**Note**: at the moment, toggling untyped highlights incurs a full restart of
Sorbet. This limitation should be removed in the future.

### In VS Code

1. Install version 0.3.19 (or later) of the
   [Sorbet VS Code extension](vscode.md).

2. Open a Ruby file, and run the `Sorbet: Toggle highlighting untyped code`
   command from the command pallet (accessed via ⇧⌘P on macOS, or ⌃⇧P on Windows
   and Linux).

This setting should persist through restarts of VS Code.

### In other LSP clients

This feature relies on the `initializationOptions` parameter to the `initialize`
request that starts
[every language server protocol session](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initialize).

To enable this feature, ensure that the `initializationOptions` includes

```json
"highlightUntyped": true
```

as a key-value pair when launching the Sorbet language server in your preferred
client.

For example, in Neovim, this setting can be provided via the [`init_options`
argument] to the `vim.lsp.start_client()` function.

[`init_options` argument]:
  https://neovim.io/doc/user/lsp.html#:~:text=initializationOptions

## Notes

- This feature is not enabled in `# typed: false` files. A `# typed: false` file
  is essentially a file where the entire contents would need to be underlined.
  Sorbet does not actually underline the entire content of such a file, as it
  would be too noisy.

- This feature will only display diagnostics for **open files**. VS Code scales
  poorly when language servers report a very large number number of diagnostics,
  and using this feature adds a non-trivial slowdown to Sorbet itself.

- These blue-underline diagnostics are **not type errors**. In this mode, type
  errors continue to be reported exclusively with red squiggles (at least in VS
  Code). Files that contain regions of code flagged by this editor mode will
  still type check without error according to `srb tc` at the command line.

- The diagnostics reported in this mode can be converted into proper errors
  (i.e., red-underline diagnostics) by marking the file `# typed: strong`. For
  more information, see [the docs for `# typed: strong`](strong.md).

- It may be helpful to use the filtering mechanism built into VS Code's
  "Problems" pane to filter these errors. A filter of `!7047` should hide these
  in the problems pane. ([7047](error-reference.md#7047) is the code for this
  diagnostic, and the `!` negates the filter.)

## How can I avoid using `T.untyped`?

→ [How can I avoid using `T.untyped`](strong.md#how-can-i-avoid-using-tuntyped)

## Why is this feature in beta?

This feature is currently under development, and we'd love your feedback!

Some known limitations:

- Sometimes, the region Sorbet highlights does not precisely match the code
  which is untyped.

  For example, a previous version of this feature accidentally highlighted an
  entire block (from `do` until `end`) when the value returned from the block
  was `T.untyped`. This was fixed by reporting exactly the block's return value.

  Please report cases like these!

  You can use [this Sorbet playground] as a starting point to report issues with
  this feature.

- Not all sources of `T.untyped` are accounted for.

  As we further develop this feature, **more sources of untyped will start being
  reported**. Feel free to report instances where it would be nice for Sorbet to
  highlight a region of untyped code, but know that we have less time to focus
  on these sorts of improvements at the moment.

  Importantly, things with type `T::Array[T.untyped]` or similar, where
  `T.untyped` appears in the middle of the type somewhere, are counted as typed.
  Code is only highlighted if it is entirely `T.untyped`.

  Also, untyped method parameters are not highlighted. For example:

  ```ruby
  sig { params(arg0: T.untyped).void }
  def foo(arg0); end
  foo(0)
  ```

  Sorbet does not highlight anything about the call to `foo`, even though the
  `arg0` argument is untyped.

- Sometimes, the source of untyped comes from RBI files that do not yet have
  types.

  In this case, please help by contributing better types!\
  See [this FAQ entry] for help contributing RBI improvements.

- Sometimes, the source of the untyped is Sorbet itself.

  Certain features of Ruby are hard to statically type and have historically
  been supported on a "best effort" basis by marking them `T.untyped`.

  Feel free to browse our [Untyped code milestone] to see if it looks like
  someone has already reported a bug. Otherwise, use [this Sorbet playground] to
  craft a test case and report it to us.

Again, to browse the most up-to-date known limitations with this feature, check
the [Untyped code milestone] for the Sorbet project.

[this sorbet playground]:
  https://sorbet.run/#%23%20typed%3A%20strong%0A%23%20To%20report%20an%20issue%2C%20click%20%22Examples%20%E2%98%B0%20%3E%20Create%20issue%20with%20example%22%0A%0AT.unsafe%28nil%29.foo
[this faq entry]: faq#it-looks-like-sorbets-types-for-the-stdlib-are-wrong
[untyped code milestone]: https://github.com/sorbet/sorbet/milestone/20
