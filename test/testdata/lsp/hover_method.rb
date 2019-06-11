# typed: true
class Foo
  extend T::Sig

  sig {returns(Integer)}
  def bar
    # ^ hover: sig {returns(Integer)}
    baz("1")
   # ^ hover: Foo
  end

  sig {params(arg0: String).returns(Integer)}
  def baz(arg0)
    arg0.to_i
  end
end
