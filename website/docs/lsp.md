---
id: lsp
title: Sorbet Language Server
sidebar_label: Language Server (LSP)
---

Sorbet implements the [Language Server Protocol], which allows using Sorbet
within virtually any text editor.

[Language Server Protocol]:
  https://microsoft.github.io/language-server-protocol/

The recommended way to interface with Sorbet's language server is via the
[Sorbet VS Code extension](vscode.md), but it's possible to use Sorbet with
other language clients.

## Prerequisites

### Watchman

For the best experience, Sorbet requires
[Watchman](https://facebook.github.io/watchman/), which listens for changes to
the files on disk in addition to edits that happen to files open in the editor.
For example, without Watchman installed, Sorbet will not detect when files have
changed on disk due to things like changing the currently checked out branch.

There are installation instructions for installing watchman on various platforms
in the [Watchman docs](https://facebook.github.io/watchman/docs/install.html).

To get Sorbet working with watchman, there are three options:

1.  Simply install the `watchman` binary somewhere visible via the `PATH`
    environment variable. Sorbet will automatically discover it.

1.  Install the `watchman` binary anywhere, and specify the path to it using the
    `--watchman-path` command line flag to Sorbet (pass it whenever you pass the
    `--lsp` flag according to the next section).

1.  Opt out of using watchman, with the `--disable-watchman` flag. Sorbet will
    only read files from disk at startup, and afterwards will only see contents
    of files that have been opened or changed in the editor.

"Why watchman?" See [A note on watchman](#a-note-on-watchman) below.

### A single input directory

While the `srb tc` command accepts any number of files and folders (and will
type check all provided files), Sorbet in LSP mode requires a single input
directory.

This is an artificial limitation that we hope to lift in the future.

The best way to workaround this is to use one or more `--ignore` flags to ignore
folders that should be excluded. For example, given a project like this, where
you only want to typecheck `lib/` and `test/` but not `bin/`:

```
.
├── bin/
├── lib/
└── test/
```

Use this in your `sorbet/config` file:

```
--dir=.
--ignore=/bin
```

instead of this, which will **not** work in LSP mode:

```
# ❌ will not work ❌
--dir=lib
--dir=test
```

Using `--ignore` flags like this is imperfect and will not always work, but it's
the best current workaround. For more, see
[Including and excluding files](cli.md#including-and-excluding-files).

## Using Sorbet as a language server

With the [Prerequisites](#prerequisites) out of the way, simply add the `--lsp`
flag to however you invoke Sorbet right now. Some examples:

```bash
srb tc --lsp

# ... if you use Sorbet via bundler:
bundle exec srb tc --lsp

# ... if you've built Sorbet from source:
bazel-bin/main/sorbet --lsp

# ... if you need to pass other LSP-related flags:
srb tc --lsp --watchman-path=/path/to/watchman
```

The final step is to teach your language client to run this command whenever
inside a Sorbet project. Check to see if
[instructions for your language client](#instructions-for-specific-language-clients)
are below, or consider contributing!

## Other useful LSP-related flags

A short list of useful LSP-related command line flags:

- `--dir=...`

  Sorbet in LSP mode requires a single input directory. The language server
  protocol has support for multiple workspace root directories
  (`workspaceFolders`) but Sorbet does not support this yet.

- `--ignore=...`

  Ignores certain paths. The `...` is a file pattern in a Sorbet-specific
  format. See
  [Including and excluding files](cli.md#including-and-excluding-files) or
  `srb tc --help` for more information.

- `--disable-watchman` / `--watchman-path=...`

  Configure whether or how the `watchman` binary is invoked.

- `--enable-all-beta-lsp-features`

  Enable all language server features the Sorbet team considers ready for open
  beta. The set of features changes over time. Please give your feedback!

- `--enable-all-experimental-lsp-features`

  Enable all language server features, even those that might be unreliable or
  cause Sorbet to crash frequently. Please give us your feedback!

- _assorted `--enable-experimental-lsp-*` flags_, (see `srb tc --help`)

  In addition to enabling all beta or all experimental LSP features, most
  work-in-progress LSP features have their own command line flag to enable that
  feature and no others.

  Note that these flags are removed when the feature is finalized, which can
  cause Sorbet to fail to start (unrecognized command line flag).

- `--lsp-error-cap`

  Certain language clients deal poorly with large quantities of diagnostics
  (errors, warnings, information hints, etc.). Sorbet caps the number of
  diagnostics it sends to clients at 1,000 diagnostics, but this can be changed
  (set it to `0` to remove the cap).

For all Sorbet flags, be sure to check `srb tc --help`.

## Instructions for specific language clients

Each language client will have its own mechanism for starting the language
server.

### Neovim

The [nvim-lspconfig] project includes support for Sorbet. Add this to your
`init.lua` file:

[nvim-lspconfig]: https://github.com/neovim/nvim-lspconfig

```lua
local lspconfig = require('lspconfig')
lspconfig.sorbet.setup {}
```

The default command is `srb tc --lsp`. To tweak the command used to start the
server, set the `cmd` property:

```lua
local lspconfig = require('lspconfig')
lspconfig.sorbet.setup {
  cmd = { "bundle", "exec", "srb", "tc", "--lsp" },
}
```

To specify custom [`initializationOptions`](#initialize-request), set the
`init_options` property:

```lua
local lspconfig = require('lspconfig')
lspconfig.sorbet.setup {
  cmd = { "bundle", "exec", "srb", "tc", "--lsp" },
  init_options = {
    highlightUntyped = true,
  },
}
```

### Other clients

If you use Sorbet with a particular language client and want to help out, please
edit this page to add instructions!

## Sorbet-specific LSP extensions

Sorbet includes a handful of custom extensions to the language server protocol.

The reason we recommend the [Sorbet VS Code extension](vscode.md) is that these
custom extensions are supported out-of-the-box when using that client.

Taking advantage of Sorbet's custom extensions in other clients is possible, but
requires some manual configuration work.

### `initialize` request

Sorbet recognizes the following options in the `initializationOptions` of the
[`initialize`] LSP request:

```typescript
export interface SorbetInitializationOptions {
  highlightUntyped?: boolean | string;
  enableTypedFalseCompletionNudges?: boolean;
  supportsOperationNotifications?: boolean;
  supportsSorbetURIs?: boolean;
}
```

- `highlightUntyped`

  Whether to [highlight usages of untyped](highlight-untyped.md) even outside of
  `# typed: strong` files. Default: `"everywhere"`

- `enableTypedFalseCompletionNudges`

  Whether to show a notice explaining when Sorbet refuses to provide completion
  results because a file is `# typed: false`. Default: `true`

  See [`# typed: false` nudges](lsp-typed-level.md#-typed-false-nudges)

- `supportsOperationNotifications`

  See [Showing the Language Server Status](server-status.md#api)

- `supportsSorbetURIs`

  See [Working with Synthetic or Missing Files](sorbet-uris.md)

### `workspace/didChangeConfiguration` notification

Sorbet recognizes the same options in the `settings` field of the
[`DidChangeConfigurationParams`] as it recognizes in the `initializationOptions`
of the [`initialize` request](#initialize-request) above.

See the definition of [`SorbetInitializationOptions`](#initialize-request)
above.

[`DidChangeConfigurationParams`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#didChangeConfigurationParams

### `sorbet/showOperation` notification

The Sorbet VS Code extension gives an indication of whether Sorbet is currently
“Idle,” meaning it has finished all requested work and is waiting, or whether
there is a current long-running operation which may affect things like whether
all the errors have been reported and whether Sorbet is able to respond to
certain editor commands.

This feature is powered by the custom `sorbet/showOperation` LSP
server-to-client notification:

_Notification_:

- method: `sorbet/showOperation`
- params: `SorbetShowOperationParams` defined as follows

```typescript
interface SorbetShowOperationParams {
  /**
   * A stable identifier for this operation. At time of writing, this includes:
   *
   * - "Indexing"
   * - "SlowPathBlocking"
   * - "SlowPathNonBlocking"
   * - "FastPath"
   * - "References"
   * - "SymbolSearch"
   * - "Rename"
   * - "MoveMethod"
   *
   * Use this field if it's important to programmatically detect a certain state
   * in the language client, as these identifiers are stable and unlikely to change.
   */
  operationName: string;

  /**
   * An unstable, human-readable description of a given operation. We reserve
   * the right to change these descriptions in future versions of Sorbet, so to
   * programmatically detect them, use `operationName`.
   *
   * Use these descriptions to show the user what operation is currently ongoing.
   * This field contains the operation descriptions documented above, like
   * "Typechecking..." instead of the technical "SlowPathBlocking" identifier.
   */
  description: string;

  /**
   * Operations can overlap. For example, a "References" operation can overlap
   * with a "SlowPathNonBlocking" operation. This field indicates whether an
   * operation is starting or ending, so the language client can track
   * overlapping operations.
   *
   * Typically language clients will display only the most recently received
   * operation notification as the current active operation. For example, if a
   * SlowPathNonBlocking operation is ongoing and a new References operation
   * begins, the client will show "Finding all references..." until that
   * operation ends, at which point the client will once again display
   * "Typechecking in background..." (until the SlowPathNonBlocking operation
   * also ends).
   *
   * When there is no active operation, language clients typically display
   * something indicating that the process is idle, like "Idle" or ✅ or
   * something else.
   *
   * There will only be one active operation for a given `operationName` at any
   * given time.
   */
  status: SorbetOperationStatus;
}

export type SorbetOperationStatus = 'start' | 'end';
```

### `sorbet/showSymbol` request

The Sorbet VS Code extension has a context menu item called "Copy Symbol to
Clipboard," which copies the fully-qualified name of the thing currently under
the cursor. For example:

```ruby
class A
  class B
    class C
      #   ^ Copy Symbol to Clipboard
    end
  end
end
```

Given this snippet, VS Code puts `A::B::C` on the system clipboard.

This feature is powered by the custom `sorbet/showSymbol` LSP request:

_Request_:

- method: `sorbet/showSymbol`
- params: [`TextDocumentPositionParams`]

_Response_:

- result: [`SymbolInformation`] | `null`

### `sorbet/readFile` request

See [Working with Synthetic or Missing Files](sorbet-uris.md) for more
information.

_Request_:

- method: `sorbet/readFile`
- params: [`TextDocumentIdentifier`]

_Response_:

- result: [`TextDocumentItem`]

## Appendix

### A note on watchman

Recall that file watching is important in the first place because certain file
edits may come from files on disk changing despite not being open in the client
itself (for example, doing a `git pull`, or editing files with some script).

The language server protocol includes support for client-side file watching as
of [version 3.17]. Sorbet does not use this functionality, but might in the
future. Instead, Sorbet does file watching with watchman.

Sorbet's language _server_ is designed so that it can be run on a separate host
from where the language _client_ runs. For example, the language client (VS
Code) can be running on a laptop, while the server process can be started via an
SSH connection to a remote host, communicating back and forth to the laptop via
the SSH connection's stdin/stdout pipe:

```bash
ssh some.remote.host -- 'cd project; bundle exec srb tc --lsp'
```

In setups like these, certain files in the codebase may not actually exist
according to the client. For example, certain generated files may only be
generated on the remote host, not the laptop. The language client would not
notice changes to these generated files, but a watchman binary running on the
same remote host as the Sorbet language server would.

Sorbet may add support for LSP-powered file watching in the future (to make
Sorbet easier to use when not using Sorbet over SSH like this). But as
specified, the current protocol is not powerful enough to completely replace
Sorbet's need for watchman.

Also, watchman uses an algorithm to find the "project path" when setting up file
watches. This algorithm is
[described here](https://facebook.github.io/watchman/docs/cmd/watch-project#whats-a-project-path)
in detail, but basically reduces to "there must be either a `.git`, `.hg`,
`.svn`, or `.watchmanconfig` file in the watched folder or some parent folder."
This list of project indicators is not configurable by Sorbet--it can only be
configured globally for an entire machine. See
[`root_files`](https://facebook.github.io/watchman/docs/config#root_files) and
[Resolution / Scoping](https://facebook.github.io/watchman/docs/config#resolution--scoping)
for more.

Thus, to use Sorbet with watchman in a project that does not use Git, create an
empty `.watchmanconfig` file in the root of the project.

[version 3.17]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#version_3_17_0
[`initialize`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initialize
[`TextDocumentPositionParams`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams
[`SymbolInformation`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolInformation
[`TextDocumentIdentifier`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentIdentifier
[`TextDocumentItem`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentItem
