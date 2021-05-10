# typed: true

class B
  def bar
    puts 'hello'
  end
end

class A
  extend T::Sig

  def baz
    B.bar
  end
end
