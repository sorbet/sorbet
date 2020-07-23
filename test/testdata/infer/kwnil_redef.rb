# typed: true

class A
  def self.foo(*args, **nil)
  end

  def self.foo(*args, mode: false) # error: Method `A.foo` redefined without matching argument count. Expected: `1`, got: `2`
  end
end

class B
  def self.bar(*args, mode: false)
  end

  def self.bar(*args, **nil) # error: Method `B.bar` redefined without matching argument count. Expected: `2`, got: `1`
  end
end

A.foo(mode: 10)
B.bar(mode: 10) # error: No keyword accepted for method `B.bar`
