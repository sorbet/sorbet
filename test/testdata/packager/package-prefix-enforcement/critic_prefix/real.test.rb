# typed: strict

module Critic::SomePkg::Real
  #    ^^^^^^^^^^^^^^^^^^^^^ error: Tests in the `Critic::SomePkg` package must define tests in the `Test::Critic::SomePkg` namespace
  FOO = 1
end

 Critic::SomePkg::RealConst = 2
#^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Tests in the `Critic::SomePkg` package must define tests in the `Test::Critic::SomePkg` namespace

# Allowed
Test::Critic::SomePkg::SomeTestConst = 3
