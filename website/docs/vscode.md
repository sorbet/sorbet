---
id: vscode
title: Extension for Visual Studio Code
sidebar_label: Visual Studio Code
---

The
[Sorbet extension for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=sorbet.sorbet-vscode-extension)
integrates with the Sorbet language server to provide IDE-like features for
typed Ruby files.

## Installing and enabling the Sorbet extension

Install the
[Sorbet extension from the VS Code extension marketplace](https://marketplace.visualstudio.com/items?itemName=sorbet.sorbet-vscode-extension).
Then, add the following configuration to your workspace's `settings.json`:

```JSON
"sorbet.enabled": true
```

The next time you open a Ruby file in the workspace, Sorbet will automatically
try to run via the following command:

```
bundle exec srb typecheck --lsp
```

If needed, you can customize how the extension launches Sorbet via the
`sorbet.lspConfigs` setting:

```json
"sorbet.lspConfigs": [{
    "id": "stable",
    "name": "Sorbet",
    "description": "Stable Sorbet Ruby IDE features",
    "cwd": "${workspaceFolder}",
    "command": [
      "bundle",
      "exec",
      "srb",
      "typecheck",
      "--lsp"
    ]
}]
```

Once Sorbet is activated, it will display its status in VS Code's status line.
For example, this is what you will see when Sorbet is busy typechecking your
latest edits:

![](/img/lsp/typechecking.gif)

For the best experience, Sorbet requires
[Watchman](https://facebook.github.io/watchman/), which listens for changes to
the files on disk in addition to edits that happen to files open in the editor.
For example, without Watchman installed, Sorbet will not detect when files have
changed on disk due to things like changing the currently checked out branch.

Sorbet simply requires that the `watchman` binary be somewhere visible on the
`PATH` environment variable. There are installation instructions for various
platforms in the
[Watchman docs](https://facebook.github.io/watchman/docs/install.html).

If you cannot install `watchman` to somewhere on the `PATH`, you can use the
`--watchman-path=...` command line flag to `srb tc` to specify a path to the
`watchman` binary.

If you cannot install `watchman` at all, pass the `--disable-watchman` flag to
`srb tc`. This will mean that Sorbet only reads the files from disk at startup,
and afterwards only ever sees contents of files that have been opened or changed
in the editor.

You can use the `sorbet.lspConfigs` setting described above to have the VS Code
extension always pass these command line flags when starting Sorbet.

## Features

Live error squiggles for Sorbet typechecking errors
([demo](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28a%3A%20String%29.void%7D%0Adef%20foo%28a%29%3B%20end%0A%0Afoo%2810%29)):

<img src="/img/lsp/errorsquiggle.png" width="75%"/>

Type information and documentation on hover
([demo](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0A%23%20Documentation%20strings%20can%20use%20_markdown_%0A%23%20*%20That%20includes%20*lists*!%0A%23%0A%23%20Tables%20also%20work%3A%0A%23%0A%23%20%7C%20Column%201%20%7C%20Column%202%20%7C%0A%23%20%7C%20-----%20%7C%20-----%20%7C%0A%23%20%7C%20True%20%20%20%7C%20*False*%20%20%20%7C%0Asig%20%7Breturns%28String%29%7D%0Adef%20my_function%0A%20%20%20%20%22%22%0Aend%0A%0A%0A%0A%0A%0A%0A%0A%0Amy_function%0A%23%20%5E%20hover%20here!)):

<img src="/img/lsp/hover.png" width="50%"/>

Go to definition / type definition
([demo](https://sorbet.run/#%23%20typed%3A%20true%0A%0A%23%20Both%20lead%20here!%0Aclass%20MyClass%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20def%20foo%3B%20end%0Aend%0A%0Amy_class_instance%20%3D%20MyClass.new%0A%23%20%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%20go%20to%20type%20definition%0A%23%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%5E%5E%5E%5E%5E%5E%5E%20go%20to%20definition%0A)):

<video autoplay muted loop width="35%" style="display:block;margin-left:auto;margin-right:auto;">
    <source src="/img/lsp/go_to_def.mp4" type="video/mp4">

    Sorry, your browser doesn't support embedded videos.

</video>

Find all references
([demo](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Parent%0A%20%20def%20foo%3B%20end%0Aend%0A%0Aclass%20NotParent%0A%20%20def%20foo%3B%20end%0Aend%0A%0Aclass%20Child1%20%3C%20Parent%3B%20end%0Aclass%20Child2%20%3C%20Parent%3B%20end%0A%0AParent.new.foo%0A%23%20%20%20%20%20%20%20%20%20%20%5E%5E%5E%20right%20click%20and%20%22Find%20All%20References%22%20here)):

<img src="/img/lsp/references.png" width="75%"/>

Autocomplete, including sig suggestion
([demo](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0A%23%20V%20type%20'g'%20here%20and%20accept%20the%20'sig'%20autocomplete%20with%20tab%0Asi%0Adef%20foo%28a%2C%20b%29%0A%20%20%22%23%7Ba%7D%20%23%7Bb%7D%22%0Aend%0A)):

<video autoplay muted loop width="70%" style="display:block;margin-left:auto;margin-right:auto;">
    <source src="/img/lsp/autocomplete_sig.mp4" type="video/mp4">

    Sorry, your browser doesn't support embedded videos.

</video>

Rename constants and methods
([demo](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20Parent%0A%20%20%23%20%20%20%5E%20Rename%20me!%0A%20%20def%20foo%3B%20end%0A%20%20%23%20%20%20%5E%20Rename%20me!%0Aend%0A%0Aclass%20Klass%20%3C%20Parent%0A%20%20def%20foo%3B%20end%0Aend%0A%0AKlass.new.foo%0AParent.new.foo%0A)):

<video autoplay muted loop width="30%" style="display:block;margin-left:auto;margin-right:auto;">
    <source src="/img/lsp/rename.mp4" type="video/mp4">

    Sorry, your browser doesn't support embedded videos.

</video>

Quick fixes (autocorrects) on errors
([demo](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Breakfast%3B%20end%0A%0ABreekfast.new%0A%23%20%5E%5E%5E%5E%5E%5E%20put%20cursor%20here%20and%20hit%20cmd%20%2B%20.%20%28Mac%29%20or%20ctrl%20%2B%20.%0A)):

<img src="/img/lsp/quickfix.png" width="40%"/>

Workspace symbol search:

<img src="/img/lsp/symbolsearch.png" width="75%"/>

Custom extension: Copy Symbol to Clipboard

<video autoplay loop muted playsinline width="597">
  <source src="/img/copy-symbol.mp4" type="video/mp4">
</video>

(If you are not using the Sorbet VS Code, you can reimplement this feature in
your preferred LSP client using the [`sorbet/showSymbol` LSP request].)

[`sorbet/showsymbol` lsp request]:
  https://github.com/sorbet/sorbet/blob/ec02be89e3d1895ea51bc72464538073d27b812c/vscode_extension/src/LanguageClient.ts#L154-L179

Highlight `T.untyped` code. This feature is in beta.

This feature reports diagnostics to the editor for occurrences of `T.untyped`
code. Note that it is not yet perfect and may miss occurrences of such values.

It can be enabled by adding the following to your VS Code `settings.json` and
either reopening VS Code or restarting Sorbet.

```json
"sorbet.highlightUntyped": true
```

or by using the `Sorbet: Toggle Highlight untyped values` command from the
command palette (note this causes a full restart of Sorbet).

To enable this feature in other language clients, configure your language client
to send

```json
"initializationOptions": {
  "highlightUntyped": true
}
```

when sending the LSP initialize request to the Sorbet language server.

<img src="/img/lsp/highlight_untyped.png" />

## Switching between configurations

The Sorbet extension supports switching between multiple configurations to make
it easy to try out experimental features. By default, it ships with three
configurations: stable, beta, and experimental. Workspaces can specify
alternative configurations via the `sorbet.lspConfigs` setting.

Users can select between these configurations on-the-fly by clicking on "Sorbet"
in VS Code's status line and selecting "Configure Sorbet". Sorbet will then
restart in the chosen configuration. Sorbet will also remember this
configuration choice for the user's future sessions in the workspace.

## Disabling the Sorbet extension

There are multiple ways to disable the Sorbet extension depending on your goals.

- You can click on Sorbet in VS Code's status line and click on _Disable
  Sorbet_, which will immediately stop Sorbet in that workspace. You will need
  to click on _Enable Sorbet_ to reenable Sorbet in that workspace. This is a
  handy way to temporarily disable Sorbet if it is causing problems.
- Workspaces can set the `sorbet.enabled` setting to `false`, which prevents
  Sorbet from running in the workspace.

## Troubleshooting and FAQ

### Startup

#### Error: "Sorbet's language server requires a single input directory. However, 0 are configured"

This error can happen if you have not initialized Sorbet in your project. Please
[follow the instructions](adopting#step-2-initialize-sorbet-in-our-project) to
initialize Sorbet.

If initializing Sorbet in your project is not desirable or possible, an
alternative fix is to override the default extension configuration in the
project's `.vscode/settings.json` file and provide the project directory as
`"."`:

```json
"sorbet.lspConfigs": [{
    "id": "stable",
    "name": "Sorbet",
    "description": "Stable Sorbet Ruby IDE features",
    "cwd": "${workspaceFolder}",
    "command": [
      "bundle",
      "exec",
      "srb",
      "typecheck",
      "--lsp",
      "."
    ]
}]
```

#### I'm not seeing "Sorbet: Disabled" or "Sorbet: Idle" in the status bar of my VSCode window

That means the extension isn't active. Here are some steps to try:

- Did you open a Ruby file? The Sorbet extension isn't activated until you open
  at least one Ruby file in your editor.
- Make sure your VS Code window is wide enough to display the entire contents of
  the status bar.
- Ensure that you are not using VS Code's
  [Multi-root Workspaces](https://code.visualstudio.com/docs/editor/multi-root-workspaces)
  feature, which this extension does not support.

#### Sorbet keeps restarting.

Click on _Sorbet_ in the status bar, and then click on _View Output_. A log
should pop up, and will typically contain some error messages that are causing
the restart.

If you see an error that looks like this:

```
[Error - 9:36:32 AM] Connection to server got closed. Server will not be restarted.
Running Sorbet LSP with:
    bundle exec srb typecheck --lsp
Could not locate Gemfile or .bundle/ directory
```

...then you probably need to run `bundle install` to install Sorbet.

### Diagnostics / error squiggles

#### Sorbet is reporting diagnostics (error squiggles) that don't make sense.

If the errors are persistent, and you can reproduce your problem in the sandbox
at https://sorbet.run/, then you've found an issue with Sorbet in general, not
necessarily the VS Code Sorbet extension. Please file a bug tagged with "IDE" on
the [issue tracker](https://github.com/sorbet/sorbet/issues).

If the errors are not persistent:

- First, is Sorbet still typechecking? If it is, wait for the status bar to
  display "Sorbet: Idle." before judging the diagnostics.
- If the diagnostics remain nonsensical when Sorbet is Idle, try restarting
  Sorbet (click on Sorbet in the status bar, and click on Restart Sorbet).
- If the diagnostics become fixed, then you might have discovered a bug in
  Sorbet.

If you arrive at a set of edits that mess up the diagnostics, please file a bug
on the [issue tracker](https://github.com/sorbet/sorbet/issues).

### Hover

#### When I hover on something, VS Code shows "Loading..."

Is Sorbet still typechecking or initializing? If so, this is expected behavior;
Sorbet cannot show you type information until it finishes catching up with your
edits. "Loading..." should get replaced with hover information once the status
bar displays "Sorbet: Typechecking in background" or "Sorbet: Idle".

#### Hover doesn't work / hover isn't showing information for my file

Is the file untyped (`# typed: false` or `ignore`)? Hover only works in typed
files. You will need to make your file `# typed: true`.

Does the file have a syntax error? If so, you need to resolve it before hover
will work again.

If the file is typed, Sorbet is "Idle", and hover isn't working, try to
reproduce the problem on https://sorbet.run and file a bug on the
[issue tracker](https://github.com/sorbet/sorbet/issues).

#### Hover is showing incorrect type information for something.

Does your file have a syntax error? If so, resolve it before proceeding.

If the problem persists with your syntax errors fixed, you may have found a bug
in Sorbet! Check to see if it's in the GitHub issue tracker. If you don't see
anything relevant there, try to golf the problem down to something small on
https://sorbet.run and file a bug on the
[issue tracker](https://github.com/sorbet/sorbet/issues).

#### Hover is not showing documentation that I've written / is showing incorrect documentation.

Internally, Hover uses the "Go to Definition" logic to locate documentation. If
"Go to Definition" takes you to the wrong location, then Sorbet doesn't have the
type information it needs to locate your documentation.

Did you include an empty line between your documentation and the thing you are
defining? We expect documentation to immediately precede the definition (or its
sig).

```ruby
# valid documentation location
def foo; end

# valid documentation location
sig {void}
def foo; end

sig {void}
# valid documentation location
def foo; end

# invalid documentation location

def foo; end
```

Is this an instance or class variable? The variable
[_must_ be defined with `T.let`](/docs/type-annotations#declaring-class-and-instance-variables),
and the documentation must precede the `T.let`. Otherwise, Sorbet doesn’t see
that variable as having ever been defined.

Otherwise, try to reproduce the issue on https://sorbet.run/ and file a bug on
the [issue tracker](https://github.com/sorbet/sorbet/issues).

### Go to Definition/Go to Type Definition/Find all References

#### Go to Definition/Go to Type Definition/Find all References is not working / Find all References is missing some expected results.

First, make sure that Sorbet is running. You should see "Sorbet: Idle" in VS
Code's status bar.

It's possible that the feature is working as intended. Go to Definition and Find
all References are not available in all circumstances. A ‘Yes’ entry in the
following table indicates two distinct things:

- A _language construct_ where these features work. For example, you can right
  click a class reference and go to its definition or find its references in
  typed and untyped files.
- An item that will be included in the results of these features. For example,
  the result of Find all References on a method includes method calls in typed
  files, but not method calls in untyped files.

|                                               | `# typed: true` or above | `# typed: false`  |
| --------------------------------------------- | ------------------------ | ----------------- |
| Class/module/constant definition or reference | Yes                      | Yes               |
| ivar (@foo)                                   | Yes, if defined\*        | Yes, if defined\* |
| cvar (@@foo)                                  | Yes, if defined\*        | Yes, if defined\* |
| Method definition                             | Yes                      | Results only\*\*  |
| Method call                                   | Yes                      | No                |
| Local variable                                | Yes                      | No                |

\* Indicates that the variable
[_must_ be defined with `T.let`](/docs/type-annotations#declaring-class-and-instance-variables).
Otherwise, Sorbet doesn’t see that variable as having ever been defined.

\*\* You cannot use either feature from this language construct, but this item
will be included in Go to Definition/Find all References results from typed
files.

We have these restrictions in place to avoid weird/nonsensical behavior caused
by untyped files, which may have partial and potentially incorrect type
information. We heartily encourage you to type files to gain access to these
features.

Note that some items marked "Yes" in the table may not work with these features
if Sorbet does not have the necessary type information. In particular, method
calls on an object `foo` where `foo` is untyped will not be included in Find all
References and will not work with Go to Definition because Sorbet is unable to
resolve which method is being called at that location. Using `# typed: strict`
should suss out most of these untyped locations on a per-file basis.

#### Find all References is slow.

The speed of Find all References depends on how many files contain an identifier
with the same name, as we use that information as a first-pass filter over the
source files before refining to actual references. For example, searching for
references to a class's `initialize` will involve a scan over most files in a
project even if the specific `initialize` you are looking for is only used in
one file.

Find all References also waits for "Typechecking in background..." to complete
so that it does not contend with typechecking for CPU time.

#### Go to Definition/Go to Type Definition brought me to what I believe is the wrong location.

Ensure that you see "Sorbet: Idle" and not "Sorbet: Disabled" at the bottom of
VS Code. If Sorbet is enabled and it is returning a weird/unexpected definition
site, please try to reproduce the issue on https://sorbet.run/ and file a bug on
the [issue tracker](https://github.com/sorbet/sorbet/issues).

#### Go to Definition/Go to Type Definition/Find all References brought me to a file that I cannot edit.

These features may return results in type definitions for core Ruby libraries,
which are baked directly into the Sorbet executable and are not present on the
file system.

In order to display these files in your editor and to support navigating through
them, we've configured the Sorbet extension to display them in this read-only
view. Note that certain extension features, like hover and Go to Definition,
will not function in some of these special files.

### Completion

#### I don't see any completion results.

- Are you in a `typed: false` file? No completion results are expected.
- Is the place where you're trying to see results unreachable? For example,
  after a return statement, or in an else condition that can't happen? Sorbet
  can't provide completion results here.
- Can you see completion results for other things? Sorbet only supports
  completing local variables, methods, keywords, suggested sigs, classes,
  modules, and constants right now. Notably, it doesn't support completing the
  names of instance variables.

#### I don't see any completion results right after I type A:: or x.

You'll have to type at least one character after the dot (like x.f) or after the
colon (like A::B) before completion results show up.

We tried to get this working before the initial ship, but it ended up being a
more complicated change than we expected. We have a couple ideas how to support
this, so expect this to be supported in the future.

#### The completion results look wrong.

Completion results can come from many different extensions, not just Sorbet. You
can try to figure out what extension returned the results by looking at the icon
that VS Code shows in the completion list:

![](/img/lsp/vscode-completion-list.png)

Results from Sorbet will only ever have 1 of 6 icons (currently): `method`,
`variable`, `field`, `class`, `interface`, `module`, `enum`, `keyword`, and
`snippet`.

**Notably**, the abc icon (`word`) means the results came either from VS Code’s
`editor.wordBasedSuggestions` setting or some other generic autocomplete
extension.

Also, `snippet` results can come from other extensions. Snippet results that
come from Sorbet will always say `(sorbet)` somewhere in the snippet
description. Sorbet does not have control over any snippet results that don't
say `(sorbet)` in them; if they look wrong, the only suggestion is to turn them
off.

#### Can I have Sorbet only suggest method names, not the entire snippet, with types?

Sorbet inserts a suggested snippet into the document when accepting a completion
result.

- Snippet results will have highlighted sections inside them.
- These represent "holes" (tabstops) that you'll need to fill in—the aim is that
  every tabstop is for a required argument (i.e., optional / default arguments
  won't be present).
- As the default text for each of these holes, Sorbet uses the type of the
  corresponding argument.
- Press `TAB` to cycle through the holes (tabstops), or press `ESC` to deselect
  all the tabstops.

It is not possible to opt-out of these completion snippets. If you find that
this is annoying, please let us know.

## Reporting metrics

> Sorbet does not require metrics gathering for full functionality. If you are
> seeing a log line like "Sorbet metrics gathering disabled," Sorbet is working
> as intended.

It is possible to ask the Sorbet VS Code extension to collect and report usage
metrics. This is predominantly useful if you maintain a large Ruby codebase that
uses Sorbet, and want to gather metrics on things like daily users and editor
responsiveness.

To start gathering metrics, implement a
[custom VS Code command](https://code.visualstudio.com/api/extension-guides/command#creating-new-commands)
using the name `sorbet.metrics.getExportedApi`.

The implementation of this command should simply return an object like this:

```js
{
  metricsEmitter: ...
}
```

The `metricsEmitter` value should conform to the [`MetricsEmitter` interface]
declared in the Sorbet VS Code extension source code. Most likely, you will want
to implement this interface by importing a StatsD client, connecting to an
internal metrics reporting host, and forwarding requests from the
`MetricEmitter` interface function calls to the StatsD client of your choice.

[`metricsemitter` interface]:
  https://github.com/sorbet/sorbet/blob/2b850340e9bccd689d6a976cddbbfecf533933ae/vscode_extension/src/veneur.ts#L11
