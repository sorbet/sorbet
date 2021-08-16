# frozen_string_literal: true
# typed: strict

class B::BClass
  extend T::Sig

  sig {returns(A::AClass)}
  def get_a
    A::AClass.new()
  end
end
