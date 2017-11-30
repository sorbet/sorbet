# @typed
class F
  def foo
    yield
  end
  def bla
    foo do
       def bar(baz)
       end
    end
  end
end
