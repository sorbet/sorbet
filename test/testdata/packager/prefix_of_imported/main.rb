# frozen_string_literal: true
# typed: strict

module Opus::Main
  p(Opus::OuterPackage)
  # ^^^^^^^^^^^^^^^^^^ error: `Opus::OuterPackage` resolves but is not exported from `Opus::OuterPackage` and `Opus::OuterPackage` is not imported

  p(Opus::OuterPackage::InnerPackage)

  p(Opus::OuterPackage::InnerPackage::SomeClass)

  p(Opus::OuterPackage::DoesNotExist)
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
end
