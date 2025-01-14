# frozen_string_literal: true
# typed: strict

class C < PackageSpec
  strict_dependencies 'dag'
  layer 'business'

  import A # error: All of `C`'s `import`s must be `dag` or higher
  test_import B
end
