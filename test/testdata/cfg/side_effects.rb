# @typed
class Side
  def foo(cond)
    a = 1
    a.foo(a, if cond; a = true; else a = 2; end); # error: Method `foo` does not exist
  end
end
