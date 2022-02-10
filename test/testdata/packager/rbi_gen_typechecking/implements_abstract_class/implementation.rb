# frozen_string_literal: true
# typed: strict

class ImplementsAbstractClassPackage::ConcreteClass < AbstractClassPackage::AbstractClass
  extend T::Sig

  sig {override.returns(String)}
  def abstract_method
    "not actually abstract"
  end
end
