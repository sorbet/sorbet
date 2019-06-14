- always "the code", not "your code"

  - always "any problems", not "your problems"
  - prefer "we" to "you"
  - docs are a learning experience; no need to make it a matter of "Us vs Them"
  - script to lint for this

- naming conventions:

  - Sorbet (capital S, no monospace) → the whole project / team
    - sorbet (lowercase) is never allowed (unless talking specifically about the
      name of the binary that we ship with `sorbet-static`)
  - `srb` → _specifically_ the static system / command line
  - `sorbet-runtime` → _specifically_ the runtime system
  - `sig` or signature, never sig

- code blocks must have language, explicit plain for "no highlight"

- guides (i.e., non-reference pages) should end with a `What's next?` section
  with 2-4 links to suggested reading, relevant to reinforce what they just read
  about.

- When possible, use descriptive variable names.
- When the variable name is insignificant, be consistent with these conventions:
  - `x`, `y`, `z`: local variables
  - `@x`, `@y`, `@z`: instance variables
  - `foo`, `bar`, `qux`: methods
  - `A`, `B`, `C`: classes
  - `M`, `N`, `O`: module
  - `Parent` / `Child`: classes with inheritance relationship
- Don't e.g. use `foo` to mean a local variable, or `f` to mean an arbitrary
  method, or `A` mean an interface.
