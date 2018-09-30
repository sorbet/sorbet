# typed: true
class Test
  extend T::Helpers

  sig {params(b: T.nilable(Integer)).returns(T.untyped)}
  def bad(b)
    T.unsafe(nil) && b && b < 0
  end
end
