---
id: lsp-typed-level
title: Feature support by strictness level
sidebar_label: LSP & Typed Level
---

Some of Sorbet's LSP features will not work in [`# typed: false` files] (also,
no LSP features will work in `# typed: ignore` files).

[`# typed: false` files]: static.md

## Type inference: `# typed: false` vs `# typed: true`

All LSP features which rely on type inference will not work in `# typed: false`
files.

Recall from [Enabling Static Checks](static.md) the primary function of a
`# typed:` sigil is to control which errors are reported and which are silenced.
Sorbet still processes `# typed: false` files, but it silences some (not all)
errors detected in those files.

The biggest class of errors silenced at `# typed: false` are those relating to
**type inference**: the process of inferring types for local variables and
checking that arguments provided at a method call site match the method's
declared parameter types.

Since all type inference-related errors are silenced at `# typed: false`, Sorbet
**does not run type inference** over `# typed: false` files at all. It's not
that Sorbet makes an attempt to infer types for local variables and then
refrains from reporting those errors: it does not run inference on these files
at all.

## Support by LSP feature

The following table summarizes what to expect to work by Ruby language feature.
The "Works in `# typed: false`" column describes whether to expect LSP features
like Hover to work. For example, the table makes it clear that hovering over a
constant literal in a `# typed: false` file will show information about that
constant, but that hovering over a local variable will not.

| Ruby feature                                    | Works in `# typed: false` |
| ----------------------------------------------- | ------------------------- |
| `class`/`module` definition                     | ✅                        |
| left side of constant assignment                | ✅                        |
| constant reference                              | ✅                        |
| method definition                               | ✅                        |
| left side of instance/class variable definition | ✅                        |
| instance/class variable usage                   | ✅**†**                   |
| method call                                     | ❌                        |
| local variable assignment                       | ❌                        |
| local variable usage                            | ❌                        |

**†**: Only if the instance or class variable at the cursor is defined. Sorbet
only requires these variables to be defined in `# typed: strict` files or above,
so it's possible that there will be no results because Sorbet has silenced an
error complaining that it could not find the definition matching an instance or
class variable usage.

The table above applies to the following LSP features, which operate on the
thing under the cursor:

<!-- TODO(jez) Should we have links to these LSP features eventually? -->

- Hover (`textDocument/hover`)
- [Go to Definition](go-to-def.md) (`textDocument/definition`)
- [Go to Type Definition](go-to-def.md#definition-vs-type-definition)
  (`textDocument/typeDefinition`)
- [Go to Implementation](go-to-def.md#go-to-implementations--find-all-implementations)
  (`textDocument/implementation`)
- [Autocompletion](autocompletion.md) (`textDocument/complete`)
- Signature Help (`textDocument/signatureHelp`)
- Document Highlight (`textDocument/documentHighlight`)

It also applies to these features which operate on both the thing under the
cursor **and** references to things throughout the codebase:

- Find All References (`textDocument/references`)
- Rename Symbol (`textDocument/rename`)

For this second set of requests, the table indicates whether the results will
include usages in `# typed: false` files (in addition to whatever was under the
cursor to initiate the request). For example, Find All References on a constant
literal will find all usages of the constant, even references in
`# typed: false` files. But Find All References on a method definition will only
find calls to that method in `# typed: true` or higher files.

## Why choose to disable certain editor features?

An alternative to disabling things like Hover and Go to Definition in
`# typed: false` files would be to turn those features on, but make it clear
that the results are "best effort," because silenced type errors have gotten in
the way of accurately knowing what's under the cursor.

Choosing to disable these LSP features is intentional, for two main reasons:

- We do not want to train people to expect incorrect results from Sorbet. It can
  be hard to know when to expect an accurate result vs when to expect a "best
  effort" result.

  Consistently inaccurate results with Sorbet erodes trust in Sorbet, especially
  for new Sorbet users where much of the codebase is not yet `# typed: true`.

- We want to encourage people to upgrade their files from `# typed: false` to
  `# typed: true`. Disabling LSP features in `# typed: false` files provides a
  reminder and a nudge that upgrading from `# typed: false` to `# typed: true`
  is a good idea.

  Upgrading the file to `# typed: true` allows both catching type errors
  introduced by future edits and turning on editor features, which aligns short-
  and long-term incentives.

## `# typed: false` nudges

To make it clear when LSP features have been disabled, Sorbet usually shows some
sort of nudge.

![Screenshot of typed: false nudge for hover](/img/lsp/hover-typed-false.png)
_The message shown when hovering over a local variable in a `# typed: false`
file._

![Screenshot of typed: false nudge for autocompletion](/img/lsp/typed-false-nudge.png)
_An empty completion item when typing the name of a local variable in a
`# typed: false` file._

### Disable autocompletion nudges in `# typed: false` files

These nudges (for autocompletion specifically) can be disabled.

In VS Code, either:

- Use the `` Sorbet: Toggle the auto-complete nudge in `typed: false` files ``
  command from the command pallet.

- Set the `sorbet.typedFalseCompletionNudges` setting to `false` in the VS Code
  preferences. This changes the _default_ setting--changing the value with the
  above "Toggle" command will override the value in the preferences.

In other LSP clients, either:

- Set the `enableTypedFalseCompletionNudges` property to `false` in the
  `initialize` request.

  See [`initialize` request](lsp.md#initialize-request)

- Send a `workspace/didChangeConfiguration` notification to the server which
  includes the `enableTypedFalseCompletionNudges` property.

  See
  [`workspace/didChangeConfiguration` request](lsp.md#workspacedidchangeconfiguration-notification)
