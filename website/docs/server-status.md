---
id: server-status
title: Showing the Language Server Status
sidebar_label: Server Status
---

Sorbet implements an extension to the [Language Server Protocol] which allows
showing the current server status.

[Language Server Protocol]:
  https://microsoft.GitHub.io/language-server-protocol/

<video autoplay loop muted playsinline width="868" style="max-width: 100%;">
  <source src="/img/lsp/vscode-server-status.mp4" type="video/mp4">
</video>

<br>

Watch the status tray in the screencast above as various kinds of edits are
made.

Sorbet gives an indication of whether Sorbet is currently "Idle," meaning it has
finished all requested work and is waiting, or whether there is a current
long-running operation which may affect things like whether all the errors have
been reported and whether Sorbet is able to respond to certain editor commands.

In [Sorbet's VS Code extension](vscode.md), server statuses are shown
out-of-the-box. In other language clients, some work will need to be done to
show server status (see [API](#api) below).

## Summary of server statuses

These are the server statuses Sorbet will report, and what they mean.

| Status                        | Sorbet is...                                                | Responsive to IDE features? | Error list is complete? |
| ----------------------------- | ----------------------------------------------------------- | --------------------------- | ----------------------- |
| Idle                          | waiting for input                                           | ✅                          | ✅                      |
| &nbsp;                        | &nbsp;                                                      | &nbsp;                      | &nbsp;                  |
| Indexing files...             | producing ASTs for each file                                | ❌                          | ❌                      |
| Typechecking...               | resolving project-wide information                          | ❌                          | ❌                      |
| Typechecking in background... | running inference on each file                              | ✅                          | ❌                      |
| Typechecking in foreground... | running inference on each file, in a blocking way           | ❌                          | ❌                      |
| &nbsp;                        | &nbsp;                                                      | &nbsp;                      | &nbsp;                  |
| Finding all references...     | working to respond to a "Find All References" request       | ❌                          | 🤔 maybe?               |
| Workspace symbol search...    | working to respond to a "Workspace Symbol Search" request   | ❌                          | 🤔 maybe?               |
| Renaming...                   | working to respond to a request to rename a definition      | ❌                          | 🤔 maybe?               |
| Moving...                     | working to respond to a request to move a method definition | ❌                          | 🤔 maybe?               |

The "Responsive to IDE features?" column indicates whether to expect an instant
or delayed response to IDE features like hover, autocompletion, go to
definition, etc. Most phases of Sorbet cause requests for IDE features to queue
until the end of the current operation. The exceptions are the **Idle** and
**Typechecking in background...** phases, which don't cause requests to use IDE
features to queue.

The "Error list is complete?" column indicates whether the list of errors is
up-to-date, or whether Sorbet is still working on producing all the errors in a
project. The **Idle** status is essentially the only phase where this is true.
The operations that indicate Sorbet is working to respond to a user request,
like finding all references, may or may not have the full set of errors,
depending on whether the request arrived when Sorbet was already in an **Idle**
state, because those operations pause any ongoing **Typechecking in
background...** operation in service of responding to the current request
faster.

For more details, see the sections below.

### Idle

Sorbet has finished all requested work and is waiting either for changes to
files in the project or for the user to make an IDE request.

All errors have been reported throughout the entire codebase.

(Technically speaking, this status is reported by the Sorbet VS Code extension,
not the Sorbet language server process. The Sorbet language server simply
reports no active operation to represent when it is idle. See the [API](#api)
below.)

### Indexing files...

Sorbet is currently reading files from disk and parsing them into abstract
syntax trees.

This operation only happens when Sorbet initially starts up in LSP mode for the
first time. If this operation is taking a long time, it can be sped up by
passing the [`--cache-dir`](cli.md#--cache-dir-caching-parse-results) flag.

### Typechecking...

Sorbet is currently analyzing the whole codebase to determine which methods are
defined where. This includes things like resolving constant references,
finalizing the inheritance hierarchy, and recording method signatures. It does
not include things like running type inference to report type mismatch errors.

### Typechecking in background...

Sorbet has finished resolving all global information (like method definitions
and the inheritance hierarchy) and is now running type inference on the methods
in each file, in parallel.

This phase is special: only in this phase (and in the **Idle** phase) is Sorbet
able to respond to IDE requests concurrently with other work. Requests like
hover, autocompletion, and go to definition will not be blocked while this
operation is ongoing.

In this phase Sorbet will still be working to compute the list of errors in this
phase, but it will prioritize reporting errors in recently-edited files over
files that have never been edited or were edited less recently.

### Typechecking in foreground...

This is a rare status which happens when Sorbet has finished the
**Typechecking...** phase (where it is resolving global information) and has
started running type inference (like in the **Typechecking in background...**
phase). But due to a rare sequence of events, Sorbet decided to typecheck a
large number of files (over 100) on the main thread, instead of in the
background on many threads. This causes IDE requests to queue in the same way as
they do in the **Typechecking...** phase.

If this phase happens frequently, please open an issue report against Sorbet and
attempt to explain what sequence of edits causes this phase to happen.

### Request-specific statuses

This includes:

- **Finding all references...**
- **Workspace symbol search...**
- **Renaming...**
- **Moving...**

These statuses only happen in response to the corresponding user request.
Sorbet's architecture has tradeoffs, making certain operations fast and others
slow. Operations like hover and go to definition can be fast because the
information needed to answer the query is either precomputed or cheap to
recompute. Operations like finding all references are slow because Sorbet has to
compute information on the fly to answer the query, which might involve
essentially re-analyzing the codebase.

When one of these slow operations is happening, Sorbet updates its status
appropriately to indicate that requests like hover or go to definition (which
would normally be fast) will be queued in the same way that they are in the
**Typechecking...** phase.

## VS Code-specific statuses

In the Sorbet VS Code extension, there are a handful more statuses which are
managed and reported by the language client itself, not the Sorbet language
server. When integrating against the [API](#api) for this feature in other
editors and language clients, these statuses will or may need to be provided by
the client itself.

### Idle

As mentioned above, the Idle status is technically managed by the VS Code
extension, not the server process. Sorbet is idle when there are no active
operations.

### Disabled

The VS Code extension is currently disabled. See
[Disabling the Sorbet extension](vscode.md#disabling-the-sorbet-extension) for
why this is, and how to enable it.

### Initializing

The VS Code extension has started the Sorbet LSP server process, but it has not
yet reported a status yet. After starting the Sorbet process for the first time,
the first status it reports is **Indexing...**.

### Restarting

The Sorbet process either crashed and needed to be restarted, or the user
requested that it be restarted directly. If Sorbet restarts and it wasn't
explicitly requested to, there is likely a bug in Sorbet which caused it to
crash. Please report a bug to the Sorbet developers. Ideally, please include the
output logged to the file mentioned by `--debug-log-file` if Sorbet was running
with this option.

### Error

There was a VS Code-specific error. This likely represents a bug in the Sorbet
VS Code extension (not the Sorbet language server). Please check the VS Code
Sorbet logs and report an issue to the Sorbet developers.

## Why do some of my edits make Sorbet go back to "Typechecking..."?

Sorbet ingests the vast majority of edits quickly, returning to its **Idle**
state either immediately or after a few hundred milliseconds. It's able to
handle these edits by _incrementally_ updating its knowledge of what's defined
in the codebase.

Unfortunately, Sorbet's approach to incrementality is simplistic—in response to
certain edits, Sorbet is forced to give up and retypecheck the codebase from
scratch. This is when the **Typechecking...** status appears.

Today, this happens for these kinds of edits:

- Edits which change over 50 files at once.
- Edits which add a new file.
- Edits which make any change in a `__package.rb` file (only if using the
  `--stripe-packages` flag).
- Edits which produce an **unrecoverable** syntax error in the file. (Sorbet is
  able to recover from most but not all syntax errors.)
- Edits which change the class or inheritance hierarchy.
- Edits which change a definition used in over 50 files.

Notably, this list does not include these common edits:

- Edits which change code inside a method body.
- Edits which only add whitespace.
- Edits which rename non-class definitions, like methods, instance variables,
  and constant assignments.
- Edits which change method signatures or constant type annotations.

Sorbet also implements one last trick: if Sorbet is currently in the
**Typechecking...** phase in response to edit `A`, and a new edit `B` comes in
immediately after, and the combination edit `A+B` would have been able to skip
the **Typechecking...** phase, Sorbet will cancel the active **Typechecking...**
operation and reprocess the `A+B` edit as if it were a single edit. This handles
many common cases like making a change and then undoing it, or introducing an
unrecoverable syntax error and then fixing it.

It is a long-term goal of the Sorbet team to eventually make every edit handled
entirely incrementally, and never have to retypecheck a codebase from scratch
after starting up.

If you are curious to measure how often and for what reason Sorbet decided to
take the `Typechecking...` phase in a project, enable
[live metric reporting](metrics.md#reporting-metrics-directly-to-statsd) for the
project, and pay attention to these groups of metrics:

- `<statsd-prefix>.lsp.counters.lsp.updates.{fastpath,slowpath,slowpath_canceled}`
- `<statsd-prefix>.lsp.counters.lsp.slow_path_reason.*`

For a more in-depth introduction to Sorbet's approach to incrementality, see
this blog post:

[Making Sorbet More Incremental →](https://blog.jez.io/making-sorbet-more-incremental/)

(The blog post above includes screenshots of dashboards which monitor those
metrics, if you're curious.)

## API

In [Sorbet's VS Code extension](vscode.md), server statuses are shown
out-of-the-box.

In other language clients, showing the server status requires some extra work.
To configure other language clients consume these statuses, follow these steps:

### 1. Pass `supportsOperationNotifications` at initialization

Sorbet only sends server statuses if the client declares it's able to understand
them. To opt into server statuses, pass the `supportsOperationNotifications`
option during the [`initialize` request].

[`initialize` request]: lsp.md#initialize-request

```js
"initializationOptions": {
  // ...
  "supportsOperationNotifications": true,
  // ...
}
```

Different language clients will have
[different ways to specify `initializationOptions`](lsp.md#instructions-for-specific-language-clients)
when starting a language server. Consult the documentation of your editor or
language client for how to pass this option on startup in the `initialize`
request.

As a reference, it may be helpful to consult the implementation in the Sorbet VS
Code extension:

- [Passing `initializationOptions`](https://github.com/sorbet/sorbet/blob/15df026a4ffe45871809885a26142bddac5cbdde/vscode_extension/src/languageClient.ts#L50-L52)

### 2. Register a handler for `sorbet/showOperation` notifications that come from the server

With the above option set, Sorbet will periodically send notifications from the
server to the client indicating the starts and ends of operations. Note that
these are [notifications] in the same sense as `textDocument/publishDiagnostics`
is a notification: they come from the server unprompted, and are not expected to
be given a response like a request would be.

[notifications]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#notificationMessage

_Notification_:

- method: `sorbet/showOperation`
- params: `SorbetShowOperationParams` as defined in the [Sorbet-specific LSP
  extensions][sorbet/showOperation]

[sorbet/showOperation]: lsp.md#sorbetshowoperation-request

Different language clients will have different ways to register code to run when
a custom notification arrives. Consult the documentation of your editor or
language client for how to register some sort of callback or handler for custom
notifications.

As a reference, it may be helpful to consult the implementation in the Sorbet VS
Code extension:

- [Registering a handler](https://github.com/sorbet/sorbet/blob/15df026a4ffe45871809885a26142bddac5cbdde/vscode_extension/src/sorbetStatusProvider.ts#L222-L224)
- [Implementation of the handler](https://github.com/sorbet/sorbet/blob/15df026a4ffe45871809885a26142bddac5cbdde/vscode_extension/src/sorbetStatusProvider.ts#L86-L91)

## Instructions for specific language clients

Different language clients will have different ways to register code to run when
a custom notification arrives. Please feel free to contribute instructions for
how to configure your preferred language client, or instructions to use some
off-the-shelf plugin which includes support for these notifications in other
editors.

### Neovim

The LSP client built into Neovim has support for registering handlers on custom
notifications. See [`:help lsp-handler`] for documentation. At the time of
writing, the [nvim-lspconfig] project does not include support for
`sorbet/showOperation` notifications out of the box.

[`:help lsp-handler`]: https://neovim.io/doc/user/lsp.html#lsp-handler
[nvim-lspconfig]: https://github.com/neovim/nvim-lspconfig
