# typed: true

class Test::A::Helper
  def foo
    puts B::CONSTANT_FROM_B
  end
end
