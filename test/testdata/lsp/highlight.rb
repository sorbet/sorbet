# typed: true

class TestClass
    # ^^^^^^^^^ def: TestClass
  def method(a, b, &block)
           # ^ def: arga
              # ^ def: b
                  # ^^^^^ def: block
    c = a + b
  # ^ def: c
      # ^ usage: arga
          # ^ usage: b
    d = c + 3
      # ^ usage: c

    block.call
  # ^^^^^ usage: block
  end

  def method_reusing_argnames(
    a: 42,
  # ^ def: adef 2
    b: 15
  # ^ def: bdef 2
  )
  a + b
# ^ usage: adef 2
    # ^ usage: bdef 2
  end
end

# Introduced to avoid awkward indenting.
if $0 == __FILE__
  # Ensure multiple variables named 'a' are not confused, and that 'a' references
  # point back to latest assignment.
  a = 3
# ^ def: outera 1
  a = 5
# ^ def: outera 2
  c = a + 1
    # ^ usage: outera 2
  d = TestClass.new()
    # ^^^^^^^^^ usage: TestClass
  d.method(1, 2) {}
end
