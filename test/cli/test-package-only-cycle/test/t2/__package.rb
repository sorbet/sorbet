# typed: strict

class Test::T2 < PackageSpec
  test!

  strict_dependencies 'dag'
  layer 'application'

  import Test::T1
end
