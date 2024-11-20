---
id: sorbet-uris
title: Working with Synthetic or Missing Files
sidebar_label: sorbet: URIs
---

> **Note**: The [Sorbet VS Code extension](vscode.md) supports this
> out-of-the-box. This doc serves as a reference for users of
> [alternative LSP clients](lsp.md) to recreate the VS Code extension behavior
> in their preferred language client.

When typechecking a project, certain files in the project are synthetic. For
example: every [RBI file](rbi.md) which defines the Ruby standard library is not
a file on disk, but actually a blob of memory baked into the Sorbet binary
executable. Also, some files may be visible to the language server, but
[missing from the language client](lsp.md#a-note-on-watchman).

Sorbet supports a handful of extensions to the
[language server protocol](lsp.md) to enable Go to Definition to work with these
files:

<video autoplay loop muted playsinline width="865" style="max-width: 100%;">
  <source src="/img/lsp/vscode-text-document-content-provider.mp4" type="video/mp4">
</video>

<br>

The video above shows that Go to Definition on the `File.read` method (defined
in an [RBI file](rbi.md) contained inside Sorbet itself) opens like a normal,
non-modifiable file inside VS Code.

The extensions powering this are:

- The `supportsSorbetURIs` property of the [`initialize` request].
- A custom `sorbet/readFile` LSP request.
- In the [Sorbet VS Code extension](vscode.md): a
  [`TextDocumentContentProvider`] to present a Virtual Document.
- The `--lsp-directories-missing-from-client` command line flag, specifying
  which extra files the language server knows about, but are not known to the
  client, which need to be served as virtual `sorbet:` URI files. (Uncommon, but
  available)

[`TextDocumentContentProvider`]:
  https://code.visualstudio.com/api/extension-guides/virtual-documents

As seen above, [Sorbet's VS Code extension](vscode.md) supports this out of the
box.

To configure other language clients to support Go to Definition for these files,
follow these steps:

## 1. Pass `supportsSorbetURIs` at initialization

By default, Sorbet produces `https://` URIs for synthetic files in its payload,
and normal `file://` URIs for all other files (even those which are missing from
the client).

The first step is to request that Sorbet send `sorbet:` URIs for these files
instead.

In the [`initialize` request] that starts the Sorbet language server, be sure to
pass the `supportsSorbetURIs` property:

```js
"initializationOptions": {
  // ...
  "supportsSorbetURIs": true,
  // ...
}
```

Different language clients will have
[different ways to specify `initializationOptions`](lsp.md#instructions-for-specific-language-clients)
when starting a language server. Consult the documentation of your editor or
language client for how to pass this option on startup in the `initialize`
request.

Setting `supportsSorbetURIs` to `true` informs Sorbet that it can use `sorbet:`
URIs. Whenever a Go to Definition request would attempt to jump into a synthetic
or missing file, instead of sending an `https://` URI or a `file://` URI, it
will send a `sorbet:` URI.

[`initialize` request]: lsp.md#initialize-request

## 2. Use the `sorbet/readFile` request to read the virtual file

By default, the language client will not know how to open files with a `sorbet:`
URI, so the next step is to teach it.

Sorbet implements a custom LSP request method called `sorbet/readFile` which
language clients can use to get the text content of a synthetic or missing file:

_Request_:

- method: `sorbet/readFile`
- params: [`TextDocumentIdentifier`]

_Response_:

- result: [`TextDocumentItem`]

Different language clients have their own way to customize the way files with a
certain URI are opened. For example:

- VS Code provides an API called [`TextDocumentContentProvider`].
- Vim and Neovim provide [BufReadCmd] and [FileReadCmd] to customize how files
  with certain protocols are read. For more information, see [`:help Cmd-event`]

The best reference for how to use the `sorbet/readFile` command is the official
Sorbet VS Code extension's TextDocumentContentProvider implementation:

→
[vscode_extension/src/sorbetContentProvider.ts](https://github.com/sorbet/sorbet/blob/master/vscode_extension/src/sorbetContentProvider.ts)

If you have implemented something which uses the `sorbet/readFile` request,
please contribute an edit to this doc which shares a link to your code, so
others can reference it!

[BufReadCmd]: https://vimhelp.org/autocmd.txt.html#BufReadCmd
[FileReadCmd]: https://vimhelp.org/autocmd.txt.html#FileReadCmd
[`:help Cmd-event`]: https://vimhelp.org/autocmd.txt.html#Cmd-event
[`TextDocumentIdentifier`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentIdentifier
[`TextDocumentItem`]:
  https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentItem
