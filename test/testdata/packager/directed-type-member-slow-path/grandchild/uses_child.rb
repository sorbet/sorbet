# typed: strict
# frozen_string_literal: true

class Grandchild::UsesChild
  extend T::Sig

  sig {returns(Child::MyChild[Integer])}
  def self.example
    T.unsafe(nil)
  end
end
