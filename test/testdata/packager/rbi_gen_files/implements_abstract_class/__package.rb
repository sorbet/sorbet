# frozen_string_literal: true
# typed: strict

class ImplementsAbstractClassPackage < PackageSpec
  import AbstractClassPackage

  export ImplementsAbstractClassPackage::ConcreteClass
end
