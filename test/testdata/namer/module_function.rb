# typed: strict
module Funcs
  sig(x: Integer).returns(Integer)
  def f(x); x; end

  module_function :f

  sig(s: Symbol).returns(Symbol)
  module_function def g(s); s; end

  module_function

  sig(s: String).returns(String)
  def h(s); s; end
end

class C
  include Funcs

  def test_calls
    f(0)
    Funcs.f(0)
    g(:f)
    Funcs.g(:f)
    h("hello")
    Funcs.h("world")

    # Classes don't inherit the `module_function`s as static methods
    C.f # error: Method `f` does not exist
  end
end
