# frozen_string_literal: true
# typed: strict

module Foo
  extend T::Sig

  sig {void}
  def foo
    A::X
    A::X::Y
  end
end
