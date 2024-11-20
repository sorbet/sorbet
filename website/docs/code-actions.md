---
id: code-actions
title: Code Actions
sidebar_label: Code Actions
---

Sorbet supports code actions via LSP.

![](/img/suggest-sig-code-action-01.png)

In VS Code, code actions can be accessed by clicking the 💡 icon on the line
that the cursor is on. (If there are no available code actions at the cursor,
there will be no 💡 icon).

Sorbet supports a number of code actions.

## Quick Fix code actions

Sorbet produces a `quickfix` code action for every correctable error. When there
is a quick fix code action available for an error message, Sorbet includes "(fix
available)" at the end of the error message:

![](/img/lsp/fix-available.png)

Accept the code action in VS Code by placing the cursor on the error message and
waiting for the 💡 icon to appear. Other language clients may have keyboard
shortcuts for this. For example in VS Code, use `Cmd` + `.` (or `Ctrl` + `.` on
Linux).

Quick Fix code actions are one-to-one with command line autocorrects. See
[Accepting autocorrect suggestions](cli.md#accepting-autocorrect-suggestions)
for more information on autocorrects.

### Apply all fixes for file

Sorbet implements a custom code action kind which allows applying all `quickfix`
code actions in the current file. This can be useful especially when upgrading a
file from `# typed: false` to `true`, or `true` to `strict`.

For example, when upgrading a file from `# typed: false` to `# typed: true`,
many of the errors are likely to be errors arising from possibly-`nil`
references. It can be useful to simply silence all these errors at once by
adding `T.must` wherever Sorbet suggests, and then possibly revisit the
autocorrects manually to decide whether any given `T.must` is the best path
forward.

As another example, Sorbet requires signatures and type annotations for all
definitions in `# typed: strict` files. Sorbet includes autocorrects to insert
these annotations alongside the error message, so applying all fixes for a file
after upgrading to `# typed: strict` is a quick way to get rudimentary type
annotations for a file that previously had none or few.

## Refactor code actions

Sorbet has a handful of refactor code actions. These code actions are not
associated with any particular error message. They aim to automate common edits.

**Note**: Most of the refactor code actions in Sorbet are limited by the
functionality of the LSP specification. In particular, the LSP specification
provides virtually no way to prompt for user input before running a code action.

### Move method to a new module

> **Trigger**: Cursor on a singleton class method

This code action moves a singleton class method to a new module. This can be
useful to split up a module which has accreted multiple unrelated helper
methods:

<video autoplay loop muted playsinline style="max-width: calc(min(813px, 100%));">
  <source src="/img/lsp/move-method-to-new-module.mp4" type="video/mp4">
</video>

As the recording above shows, Sorbet will:

- Create a new module to hold the method, at the top of the current file.

- Move the method body to that module.

- Rename all calls to the method to use the new module's name.

This is an especially useful tool to split dependencies in heavily-tangled code.
In codebases that use an autoloader (most Rails codebases), writing
`Helpers.compute_qux` requires loading the entire `Helpers` file, which might
cause a bunch of other code to load, even if that code isn't used by the
`compute_qux` method. Splitting `compute_qux` into its own module in a separate
file can be a way to optimize code loading.

Due to limitations in the LSP specification, Sorbet cannot prompt for the name
of the new module to create, nor put it in a separate file. It chooses a new
name based on the name of the method, and inserts the module at the top of the
file. To choose a different name, use Rename Symbol to change the name after
extracting the method to its own module. (The new name can be a fully-qualified
or namespaced name, like `A::B::C`.)

<!-- TODO(jez) Document Rename Symbol, and link to it here -->

### Convert to singleton class method

> **Trigger**: Cursor on an instance method

This code action converts an instance method to a singleton class method on the
same class.

<video autoplay loop muted playsinline style="max-width: calc(min(813px, 100%));">
  <source src="/img/lsp/convert-to-singleton-class-method.mp4" type="video/mp4">
</video>

As the recording above shows, Sorbet will:

- Change the definition from `def method_name` to `def self.method_name`

- Add a new method parameter to the definition called `this` in both the method
  definition and the signature (if the method has one).

- Refactor all call sites like `x.method_name(...)` to `X.method_name(x, ...)`.

At the moment, this code action does not update the method body itself to
rewrite method calls on `self` (implicitly or explicitly) to calls on the new
`this` method parameter.

This can be useful for example to decouple logic from database models. Over time
it can be problematic to have lots of behavior and business logic accumulate on
database models. Moving instance methods to singleton class methods can be a
first step towards factoring those methods into new modules (See also: the
[Move method to a new module](#move-method-to-a-new-module) code action.)

### Delete `T.unsafe` / `T.must`

> **Trigger**: Cursor on a call to `T.unsafe` or `T.must`

This code action deletes a `T.unsafe`. Sometimes it might be interesting to see
the error that a call to `T.unsafe` or `T.must` is silencing (or to check
whether there still are any such errors.)

This code action simply changes `T.unsafe(expr)` to `expr`.

<!-- TODO(neil) Document Extract Variable eventually -->
