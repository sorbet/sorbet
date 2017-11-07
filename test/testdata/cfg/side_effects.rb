class Side
  def foo
    a = 1
    a.foo(a, if true; a = true; else a = 2; end);
  end
end
