# typed: true
extend T::Sig

class A
  private def initialize
    puts 'hello'
  end
end

a = A.new
a.initialize # error: Non-private call to private method `initialize`

class B
  def self.new
  end
  private_class_method :new
end

b = B.new # error: Non-private call to private method `new`
b.initialize

class C
  # At time of writing, Sorbet doesn't support changing visibility of an
  # inherited method.
  private_class_method :new
end

c = C.new
c.initialize # error: Non-private call to private method `initialize` on `C`

class D
  def self.initialize
  end
end

D.initialize

class E
end

sig {params(xs: T::Array[T.class_of(E)]).void}
def example(xs)
  xs.map(&:new)
end

class F
  public def initialize
  end
end
f = F.new
f.initialize
