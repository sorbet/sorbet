# typed: strict

module Test::Root::A # error: File belongs to package `Root::A` but defines a constant that does not match this namespace
  class FromLegacyTestNamespace # error: File belongs to package `Root::A` but defines a constant that does not match this namespace
  end
end
