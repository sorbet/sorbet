# frozen_string_literal: true
# typed: strict

class C < PackageSpec
  strict_dependencies 'dag'
  layer 'business'

  import A # error: All of `C`'s `import`s must be `dag` or higher
  import Nested
  #      ^^^^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)` but found `T.class_of(Nested)`
  test_import B
end
