# typed: true
class Bar
end

module Foo
  def foo(unknown)
    ret = T.let([], T::Array[Bar])
    Kernel.loop do
      Kernel.puts if unknown
      ret.push(Bar.new) if bar
    end
  end

  def bar
  end
end
