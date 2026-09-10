# typed: strict

Test::Target::Helper

module ProdNone::Nested
  module Test
    X = 1
  end

  T.let(Test::X, Integer)
end
