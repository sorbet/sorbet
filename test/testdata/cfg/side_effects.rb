# @typed
class Side
  def foo
    a = 1
    a.foo(a, if true; a = true; else a = 2; end); # error: Method foo does not exist
  end
end
