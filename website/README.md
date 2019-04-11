# Writing docs

This is the source for Sorbet's documentation site. It was scaffolded with and
is currently built with [Docusaurus](https://docusaurus.io/). While some things
are documented here, most of the Docusaurus user docs lives on their site.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Table of Contents

- [Getting started](#getting-started)
- [Editing existing docs](#editing-existing-docs)
- [Adding a new doc](#adding-a-new-doc)
- [Docusaurus blog](#docusaurus-blog)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Getting started

All commands must be run from within Sorbet's top-level `website/` folder.

1.  Install the dependencies:

    ```sh
    ❯ yarn
    ```

1.  Run the dev server (watch mode):

    ```sh
    ❯ yarn start
    ```

1.  The build should be automatically deployed when pushing to `github.com`.
    To deploy manually:

    ```sh
    # Build site into website/build/sorbet
    ❯ yarn build

    # Publish
    ❯ git remote add github.com git@github.com:stripe/sorbet.git
    ❯ git checkout gh-pages
    ❯ cp -r website/build/sorbet/* .
    ❯ git add .
    ❯ git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
    ❯ git push github.com gh-pages
    ```


## Editing existing docs

External docs live in the `website/docs/` folder (make sure you're not adding
docs to the top-level `docs/` folder, which is for Sorbet internal docs).

For more information about docs, click
[here](https://docusaurus.io/docs/en/navigation).


## Adding a new doc

1.  Create a new markdown doc:

    ```sh
    ❯ touch docs/my-new-doc.md
    ```

1.  Initialize the doc with a header, and start writing!

    ```
    ---
    id: my-new-doc
    title: My New Doc
    ---

    My content...
    ```

    The filename is what will be used for your doc's URL.
    The id is what will be used to refer to your doc in the sidebar.

1.  Add the doc to the sidebar:

    [→ sidebars.json](sidebars.json)

    Docusaurus allows for multiple sidebars, so we could potentially have
    different sidebars for different major sections of the docs. For now, we
    have all the docs referenced in a single sidebar.

For more information about adding new docs, click
[here](https://docusaurus.io/docs/en/navigation).


## Docusaurus blog

Docusaurus supports having a blog, which are basically docs with release dates,
and they show up in a separate section.

We're currently not using the blog for anything, but we could use it in the
future for release notes, communicating roadmaps, etc.

For more information about blog posts, click
[here](https://docusaurus.io/docs/en/adding-blog).

## A note about logos

The canonical Sorbet logos are located in the `../docs/logo/` folder (i.e., at the
top-level of the repo). Logos in the website's `static/img/` folder are derived
(usually: copied) from there.

If you make changes to the logo and want to re-generate the `.ico` file, install
ImageMagick and then run:

```
convert ../docs/logo/sorbet-logo.svg -transparent white -define icon:auto-resize static/img/favicon.ico
```
