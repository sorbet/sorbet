# typed: strict
# frozen_string_literal: true

# stratum: 1

class Child::MyChild
  extend T::Sig

  sig {returns(Parent::ParentAlias)}
  def self.bar = raise
end
