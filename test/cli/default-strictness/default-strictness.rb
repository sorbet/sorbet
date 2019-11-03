"x" + :error

module Foo
  extend T::Sig

  sig { returns(Integer) }
  def bar
    3
  end
end
