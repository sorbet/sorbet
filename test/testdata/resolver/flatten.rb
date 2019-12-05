# typed: true
class A
  def foo
    def food
    end
    def self.foos
    end
  end
  def self.sfoo
    def sfood
    end
    def self.sfoos
    end
  end
end

A.new.foo
A.foo # error: Method `foo` does not exist on `T.class_of(A)`

A.new.food
A.food # error: Method `food` does not exist on `T.class_of(A)`

A.new.foos
A.foos # error: Method `foos` does not exist on `T.class_of(A)`


A.sfoo
A.new.sfoo # error: Method `sfoo` does not exist on `A`

A.new.sfood
A.sfood # error: Method `sfood` does not exist on `T.class_of(A)`

A.sfoos
A.new.sfoos # error: Method `sfoos` does not exist on `A`
