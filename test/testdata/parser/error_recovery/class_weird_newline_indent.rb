# typed: false

class A
  class Inner < Object
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed

  def method2
  end
end

class B
  class Inner <
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    Object

  def method2
  end
end

class C
  class Inner <
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    Object
    puts 'hello'

  def method2
  end
end

class D
  class
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    Inner

  def method2
  end
end

class E
  class
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    Inner
    puts 'hello'

  def method2
  end
end

class F
  class
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    Inner <
      Object
    puts 'hello'

  def method2
  end
end # parser-error: unexpected token "end of file"
