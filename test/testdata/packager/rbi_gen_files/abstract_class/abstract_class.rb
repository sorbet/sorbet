# frozen_string_literal: true
# typed: strict

class AbstractClassPackage::AbstractClass
  extend T::Helpers
  extend T::Sig
  abstract!

  sig {abstract.returns(String)}
  def abstract_method; end
end

