---
id: sig-suggestion
title: Automatically suggesting method signatures
sidebar_label: Suggesting sigs
---

In general, Sorbet does not attempt to infer parameter and return types for
method methods without [method signatures](sigs.md).

(For further context, see
[Why does Sorbet sometimes need type annotations?](why-type-annotations.md).)

However, Sorbet has best effort support for **suggesting** method signatures
that the user may want to insert into the codebase. By best effort we mean that
sometimes Sorbet will fail to infer any useful types. Despite limitations,
Sorbet is able to suggest signatures in many cases. There are two main ways to
get Sorbet to suggest a signature for a method.

## Quickfix code actions in `# typed: strict` or higher

It's an error in a [`# typed: strict`](static.md) file to have a method without
a signature. When reporting these errors, Sorbet runs its signature suggestion
logic to attempt to provide an autocorrect.

In editors, these [autocorrects] are surfaced as **quickfix code actions**.
Different language clients have different ways to apply these code actions, but
in VS Code they are surfaced either by clicking on the 💡 lightbulb icon, or by
pressing `Ctrl` + `.` (or `Cmd` + `.`, depending on your platform).

![](/img/suggest-sig-code-action-01.png)

[autocorrects]: cli.md#accepting-autocorrect-suggestions

However, if Sorbet's suggested signature is composed **entirely** of
`T.untyped`, it does not generate an autocorrect nor a quickfix (unless running
with the [`--suggest-unsafe` flag]). This is to avoid accidentally desensitizing
`T.untyped` for new users of Sorbet.

[`--suggest-unsafe` flag]: cli.md#silencing-errors-in-bulk

When this is the case, the only remaining option is to use completion item
snippets.

## Completion item snippets in `# typed: true` or higher

When generating autocompletion items, Sorbet notices when it's autocompleting a
call to the `sig` method, and attaches a snippet to the signature with a
suggested sig:

![](/img/suggest-sig-completion-item-01.png)

These suggestions apply anywhere that method name completion works (namely:
`# typed: true` or higher files), so it doesn't require that there already be an
error for a method lacking a signature.

In VS Code and other clients with snippet support, the snippet will have custom
tab stops. This allows pressing `TAB` to cycle through the placeholder types in
the suggested signature to fill in a proper type for each placeholder:

<video autoplay loop muted playsinline style="max-width: calc(min(962px, 100%));">
  <source src="/img/suggest-sig-completion-item-02.mp4" type="video/mp4">
</video>

## Suggesting signatures in bulk

Adding signatures to a codebase in bulk is tricky, because Sorbet does not
always suggest a suitable signature, and because adding a signature can cause
type errors to appear elsewhere.

For the brave, some options that can be useful for adding signatures in bulk are
documented in the Command Line Reference doc, including:

- `--typed <level>` and `--typed-override <file.yaml>`
  - [Docs link](cli#overriding-strictness-levels)
  - These flags are useful to temporarily override the sigil of a file. Using
    these flags to upgrade files to `# typed: strict` will mean that Sorbet's
    built in "This method does not have a sig" errors will appear, with
    autocorrects attached
- `--autocorrect` and `--isolate-error-code <code>`
  - [Docs link](cli.md#limiting-autocorrect-suggestions)
  - There are many errors reported at `# typed: strict`, but when bulk-adding
    signatures, the only autocorrect that's worth accepting is
    [7017](error-reference.md#7017).
- Optionally: `--suggest-unsafe`
  - [Docs link](cli.md#silencing-errors-in-bulk)
  - This option

In summary:

```bash
srb tc --typed=strict --isolate-error-code=7017 --autocorrect

# or, if it's okay to have entirely `T.untyped` signatures:
srb tc --typed=strict --isolate-error-code=7017 --autocorrect --suggest-unsafe
```

> **Note**: while this should generate syntactically valid code, it will likely
> cause many new errors which require manual or semi-automated intervention.
>
> For more tips on running large codemods, see
> [this blog post](https://blog.jez.io/codemods-tips/).
