# typed: true

class TestClass
    # ^^^^^^^^^ def: TestClass
  def method(a, b)
    c = a + b
    d = c + 3
  end
end

d = TestClass.new()
  # ^^^^^^^^^ usage: TestClass
d.method(1, 2)
