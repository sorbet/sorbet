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

  test_import B
# ^^^^^^^^^^^ error: Method `test_import` does not exist
  #           ^ error: Invalid expression in package
end
