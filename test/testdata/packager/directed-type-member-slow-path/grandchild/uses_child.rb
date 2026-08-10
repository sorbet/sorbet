# typed: strict
# frozen_string_literal: true
# disable-fast-path: true

class Grandchild::UsesChild
  extend T::Sig

  sig {returns(Child::MyChild[Integer])} # error: `Child::MyChild` is not a generic class, but was given type parameters
  #                           ^^^^^^^ error: `Child::MyChild` is not a generic class, but was given type parameters
  def self.example
    T.unsafe(nil)
  end
end
