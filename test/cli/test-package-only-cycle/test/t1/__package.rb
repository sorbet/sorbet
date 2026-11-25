# typed: strict

class Test::T1 < PackageSpec
  test!

  strict_dependencies 'dag'
  layer 'application'

  import Test::T2
end
