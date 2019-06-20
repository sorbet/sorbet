# typed: true
class Foo
  extend T::Sig

  sig {returns(Integer)}
  def bar
    # ^ hover: sig {returns(Integer)}
    # N.B. Checking two positions on below function call as they used to return different strings.
    baz("1")
  # ^ hover: sig {params(arg0: String).returns(Integer)}
   # ^ hover: sig {params(arg0: String).returns(Integer)}
  end

  sig {params(a: String).void}
  def self.bar(a)
         # ^ hover: sig {params(a: String).void}
  end

  sig {params(arg0: String).returns(Integer)}
  def baz(arg0)
    no_args_and_void
  # ^ hover: sig {void}
    Foo::bat(1)
  # ^ hover: T.class_of(Foo)
       # ^ hover: sig {params(i: Integer).returns(Integer)}
           # ^ hover: Integer(1)
    arg0.to_i
  end

  sig {params(i: Integer).returns(Integer)}
  def self.bat(i)
    10
  end

  sig {void}
  def no_args_and_void
  end

  sig {returns(T.noreturn)}
  def always_raises
    # ^ sig {returns(T.noreturn)}
    raise RuntimeError
  end
end
