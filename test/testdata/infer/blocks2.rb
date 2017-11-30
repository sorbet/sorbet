# @typed
class Foo
  def bar()
    yield(1);
  end

  def baz()
    bar() do |r|
      puts r # error: does not exist
    end
  end
end
