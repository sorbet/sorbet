# typed: false

class A
  class Inner < Object

  def method2
  end
end

class B
  class Inner <
    Object

  def method2
  end
end

class C
  class Inner <
    Object
    puts 'hello'

  def method2
  end
end

class D
  class
    Inner

  def method2
  end
end

class E
  class
    Inner
    puts 'hello'

  def method2
  end
end

class F
  class
    Inner <
      Object
    puts 'hello'

  def method2
  end
end
