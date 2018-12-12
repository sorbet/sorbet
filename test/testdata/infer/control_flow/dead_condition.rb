# typed: true
class Test
  extend T::Sig

  sig {params(b: T.nilable(Integer)).returns(T.untyped)}
  def bad(b)
    T.unsafe(nil) && b && b < 0
  end
end
