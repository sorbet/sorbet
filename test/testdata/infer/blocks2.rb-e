# typed: true
class Foo
  def bar()
    yield(1);
  end

  def baz()
    bar() do |r|
      puts r
    end
  end
end
