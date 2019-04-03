- always "the code", not "your code"
  - always "any problems", not "your problems"
  - prefer "we" to "you"
  - docs are a learning experience; no need to make it a matter of "Us vs Them"
  - script to lint for this

- naming conventions:
  - Sorbet (capital S, no monospace) → the whole project / team
    - sorbet (lowercase) is never allowed (unless talking specifically about the
      name of the binary that we ship with `sorbet-static`)
  - `srb` → *specifically* the static system / command line
  - `sorbet-runtime` → *specifically* the runtime system
  - `sig` or signature, never sig

- each doc should declare audience + main point in a comment
  - lint for this

- prettier
  - code blocks must have language, explicit plain for "no highlight"

- guides (i.e., non-reference pages) should end with a `What's next?` section
  with 2-4 links to suggested reading, relevant to reinforce what they just read
  about.
