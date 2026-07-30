# typed: strict

module Critic::SomePkg::Real
  #    ^^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Test::Critic::SomePkg` but defines a constant that does not match this namespace
  FOO = 1
end

 Critic::SomePkg::RealConst = 2
#^^^^^^^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Test::Critic::SomePkg` but defines a constant that does not match this namespace

# Allowed
Test::Critic::SomePkg::SomeTestConst = 3
