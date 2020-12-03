# typed: true

class A
  def outer_method
    def inner_method
      puts self
    end
  end
end

A.new.outer_method
A.new.inner_method

class B
  def outer_method
    def self.inner_method
      puts self
    end
  end
end

b = B.new
b.outer_method
b.inner_method

# B.new.inner_method # <- raises!

class C
  def self.outer_method
    def inner_method
      puts self
    end
  end
end

C.outer_method
C.new.inner_method

class D
  def self.outer_method
    def self.inner_method
      puts self
    end
  end
end

D.outer_method
D.inner_method
