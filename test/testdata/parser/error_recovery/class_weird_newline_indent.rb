# typed: false

class A
  class Inner < Object
# ^^^^^ error: Hint: this "class" token might not be properly closed

  def method2
  end
end

class B
  class Inner <
# ^^^^^ error: Hint: this "class" token might not be properly closed
    Object

  def method2
  end
end

class C
  class Inner <
# ^^^^^ error: Hint: this "class" token might not be properly closed
    Object
    puts 'hello'

  def method2
  end
end

class D
  class
# ^^^^^ error: Hint: this "class" token might not be properly closed
    Inner

  def method2
  end
end

class E
  class
# ^^^^^ error: Hint: this "class" token might not be properly closed
    Inner
    puts 'hello'

  def method2
  end
end

class F
  class
# ^^^^^ error: Hint: this "class" token might not be properly closed
    Inner <
      Object
    puts 'hello'

  def method2
  end
end # error: unexpected token "end of file"
