# typed: strict
@foo = 1 # error: The singleton class instance variable `@foo` must be declared using `T.let` when specifying `# typed: strict`
