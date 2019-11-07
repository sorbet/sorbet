# typed: true

class TestClass
    # ^^^^^^^^^ def: TestClass
  def method(a)
           # ^ def: arga
    a
  # ^ usage: arga
  end
end

# Introduced to avoid awkward indenting.
if $0 == __FILE__
  d = TestClass.new()
    # ^^^^^^^^^ usage: TestClass
end
