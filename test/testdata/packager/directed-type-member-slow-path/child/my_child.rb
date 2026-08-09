# typed: strict
# frozen_string_literal: true

class Child::MyChild
  extend T::Generic
  Elem = type_template

  extend T::Sig

  sig {returns(Integer)}
  def self.foo
    1
  end
end
