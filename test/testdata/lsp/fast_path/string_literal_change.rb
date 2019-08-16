# typed: strict
# disable-fast-path: true

A = ["bar"]
  # ^^^^^^^ error: Constants must have type annotations with `T.let` when specifying `# typed: strict`
  # ^^^^^^^ error: Suggested type for constant without type annotation: `T::Array[String]`
