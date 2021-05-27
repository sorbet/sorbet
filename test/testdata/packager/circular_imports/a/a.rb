# frozen_string_literal: true
# typed: strict

class A::AClass
  extend T::Sig

  sig {returns(B::BClass)}
  def get_b
    B::BClass.new()
  end
end
