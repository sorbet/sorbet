# typed: strict

module Test::D
  FROM_E = T.let(E::CONSTANT_FROM_E, String)
  NOT_EXPORTED = D::ShouldNotBeExported
end
