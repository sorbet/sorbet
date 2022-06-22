# typed: true

class A
  def self.foo
  end
end

A.foo
A.bar # error: Method `bar` does not exist on `T.class_of(A)`
