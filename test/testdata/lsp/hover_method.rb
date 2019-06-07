# typed: true
class Foo
  extend T::Sig

  sig {returns(Integer)}
  def bar
    # ^ hover: returns(Integer)
    baz("1")
  # ^ hover: params(arg0: String).returns(Integer)
   # ^ hover: params(arg0: String).returns(Integer)
   self.baz("1")
      # ^ hover: params(arg0: String).returns(Integer)
  end

  sig {params(arg0: String).returns(Integer)}
  def baz(arg0)
    Foo::bat
  # ^ hover: Foo
       # ^ hover: returns(Integer)
    arg0.to_i
  end

  sig {returns(Integer)}
  def self.bat()
    10
  end
end
