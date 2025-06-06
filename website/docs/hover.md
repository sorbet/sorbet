---
id: hover
title: Types and documentation on Hover
sidebar_label: Hover
---

Sorbet can show the types and documentation for parts of the program via the LSP Hover feature.

<img src="/img/lsp/hover.png" style="max-width: 346px" />

[View on sorbet.run →](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0A%23%20Documentation%20strings%20can%20use%20_markdown_%0A%23%20*%20That%20includes%20*lists*!%0A%23%0A%23%20Tables%20also%20work%3A%0A%23%0A%23%20%7C%20Column%201%20%7C%20Column%202%20%7C%0A%23%20%7C%20-----%20%7C%20-----%20%7C%0A%23%20%7C%20True%20%20%20%7C%20*False*%20%20%20%7C%0Asig%20%7Breturns%28String%29%7D%0Adef%20my_function%0A%20%20%20%20%22%22%0Aend%0A%0A%0A%0A%0A%0A%0A%0A%0Amy_function%0A%23%20%5E%20hover%20here!)

The key features:

- Shows type information

- Shows associated documentation (excludes local variables).

  See [Documentation Comments](doc-comments.md) for more information.

- Markdown support (conditional on language client)

  Consult the docs for your language client to ensure whether it requests Hover results in Markdown or plain text. Some terminal-based clients request plain text.

  (Sorbet surfaces comment content unchanged, which may or may not use Markdown syntax. It only wraps types in Markdown fenced code blocks if the client supports it.)

## Troubleshooting

### VS Code shows "Loading..."

Sorbet will only show Hover results when it is in the `Idle` or `Typechecking in background...` states.

Read about all of Sorbet's [server statuses](server-status.md) for more information on these states.

If Sorbet is in the Idle state and hovering still shows `Loading...`, that usually suggests a bug in Sorbet. Double check whether the server process crashed, or whether Sorbet is using excessive CPU cycles (Sorbet should consume virtually no CPU cycles when Idle).

### No hover results

Is the file `# typed: false` or `# typed: ignore`? Hover support is degraded in these files. Similarly, is the call site [untyped](untyped.md)? If so, consider [troubleshooting why](troubleshooting.md).

See [Feature support by strictness level](lsp-typed-level.md) to learn what's expected.

If the file is `# typed: true`, the server is in the Idle status, and there are still no results, this could either be that there is nothing under the cursor (expected), or that Sorbet mistakenly thinks there's nothing under the cursor (unexpected, please file a bug).

### Hover isn't showing documentation comments

Double check that there isn't a blank line between the documentation comment and the definition.

Otherwise, read [Documentation Comments](doc-comments.md) to confirm what the expected behavior is, and consider reporting a bug.
