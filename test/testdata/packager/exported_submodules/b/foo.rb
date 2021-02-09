# frozen_string_literal: true
# typed: strict
# enable-packager: true

module Foo
  extend T::Sig

  sig {void}
  def foo
    A::X
  # ^^^^ error: Unable to resolve constant `X
    A::X::Y
  # ^^^^ error: Unable to resolve constant `X
    A::Y
  end
end
