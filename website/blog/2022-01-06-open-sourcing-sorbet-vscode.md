---
id: open-sourcing-sorbet-vscode
title: Open-sourcing the Sorbet VS Code Extension
---

We’re excited to announce that Stripe’s VS Code extension for Sorbet is now
[open source](https://github.com/sorbet/sorbet/tree/master/vscode_extension).
We’ve designed Sorbet to be used in editors from Day 1—For the past two years,
Sorbet has exposed a flag (`--lsp`) that starts Sorbet in
[Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
(LSP) mode. In this mode, Sorbet can respond to many LSP requests, like Go To
Definition, Find All References, Autocomplete, and more.

With this release, we’re making it even easier to take advantage of these LSP
features when working with Sorbet in VS Code. We hope that this extension takes
some of the guess work out of how to configure and use Sorbet’s LSP mode.

<!--truncate-->

Today’s release includes:

- The
  [pre-built extension](https://marketplace.visualstudio.com/items?itemName=sorbet.sorbet-vscode-extension)
  on the Visual Studio Marketplace
- The
  [source code](https://github.com/sorbet/sorbet/tree/master/vscode_extension)
  for the extension, located in the `vscode_extension/` folder of the Sorbet
  repo
- Full [installation and usage instructions](https://sorbet.org/docs/vscode) in
  the Sorbet docs

## Showcase

The best way to experience the Sorbet extension for VS Code is to try it out
yourself. To entice you to try it out for yourself, here are some screen
recordings.

When there are errors in a file, Sorbet will underline them in red. It also
shows a list of all errors in VS Code’s “Problems” window.

![Sorbet errors are shown with red squiggles in the editor. Hover to see the full message.](/img/lsp/errorsquiggle_blog.png)

Sorbet responds with autocompletion results while typing, including a list of
possible methods and the documentation for that item, including its signature.

![Autocompletion works for constants, methods, and variables](/img/lsp/autocomplete_blog.png)

Sorbet can find all references of a constant, method, or variable throughout an
entire codebase:

<p><video autoplay muted loop width="100%" style="display:block;margin-left:auto;margin-right:auto;">
    <source src="/img/lsp/references_blog.mp4" type="video/mp4">
</video></p>

… and having found all those references, it can even rename them automatically:

<p><video autoplay muted loop width="100%" style="display:block;margin-left:auto;margin-right:auto;">
    <source src="/img/lsp/rename_blog.mp4" type="video/mp4">
</video></p>

## Acknowledgements

Finally, this release represents the work of many people over the last few
years, all of whom we’ve grateful for, including but not limited to:

- Ainsley Escorce-Jones
- Akshay Joshi
- Andrew O'Neil
- Anthony Pratti
- Ariel Davis
- Caleb Barde
- Christopher Brown
- Daniel Molina
- Dmitry Petrashko
- G. D. Ritter
- Ilya Zheleznikov
- Jacob Zimmerman
- Joey Pereira
- John Vilk
- Jonathan Fung
- Jérémie Laval
- Kevin Miller
- Lance Lafontaine
- Manjiri Tapaswi
- Mick Killianey
- Nathaniel Roman
- Nelson Elhage
- Patrick Vilhena
- Paul Tarjan
- Penelope Phippen
- Robb Shecter
- Soam Vasani
- Susan Tu
- Sushain Cherivirala
- Trevor Elliott
- Ufuk Kayserilioglu
- Vignesh Shankar

In particular, John Vilk has been instrumental in leading the development of the
Sorbet LSP server for the past several years. Thanks to him and all who have
contributed in the past!

As always, if you encounter issues or want to suggest features, file an issue
against
[the Sorbet issue tracker](https://github.com/sorbet/sorbet/issues/new/choose)
on GitHub, and if you want to come chat with us about Sorbet or the Sorbet VS
Code extension, [join us on the Sorbet Slack](https://sorbet.org/slack).

Thanks!\
— The Sorbet team
