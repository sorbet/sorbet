# frozen_string_literal: true
# typed: strict
# enable-packager: true

class A < PackageSpec
  import 123
       # ^^^ error: Argument to `import` must be a constant
       # ^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)`
  import "hello"
       # ^^^^^^^ error: Argument to `import` must be a constant
       # ^^^^^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)`
  import method
       # ^^^^^^ error: Argument to `import` must be a constant
       # ^^^^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)`
       #       ^ error: Not enough arguments
  import REFERENCE
       # ^^^^^^^^^ error: Unable to resolve constant `REFERENCE`
  # Despite the above, this import should work.
  import B
  import B

  export 123 # error: Argument to `export` must be a constant
  export "hello" # error: Argument to `export` must be a constant
  export method # error: Argument to `export` must be a constant
       #       ^ error: Not enough arguments
  export A::REFERENCE # Works; it's a constant.
  export A::AClass
  export A::AModule

  # `test_import` no longer has any effect, so it does not check its arguments.
  test_import B
  test_import C
  test_import C, only: :A_MYSTERIOUS_PURPOSE
  test_import C, only: "something else"
  test_import C, only: -> { "naught" }
  #                       ^^^^^^^^^^^^ error: Invalid expression in package: `Block` not allowed
  test_import C, only: "test_rb", only: "test_rb"
  #                               ^^^^ error: Hash key `only` is duplicated
  test_import C, with: "cheese"
end
