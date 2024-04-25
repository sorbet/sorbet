# typed: strict
# frozen_string_literal: true
# compiled: true

class A
  extend T::Sig

  sig {params(x: Integer, y: T.untyped).void}
  def foo(x, **y)
    puts x
    puts y
  end

end
