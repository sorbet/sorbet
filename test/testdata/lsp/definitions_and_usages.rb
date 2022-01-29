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

  def method_with_keyword_args(
    meaning_of_life: 42,
  # ^^^^^^^^^^^^^^^ def: meaning_of_life 1
                   # ^^ def: meaning_of_life 2 default-arg-value
    coolest_angle: 15
  # ^^^^^^^^^^^^^ def: coolest_angle 1
                 # ^^ def: coolest_angle 2 default-arg-value
  )
    meaning_of_life + 1
  # ^^^^^^^^^^^^^^^ usage: meaning_of_life 1,2
    coolest_angle + 1
  # ^^^^^^^^^^^^^ usage: coolest_angle 1,2
  end

  def method_with_rest_arg(*arr)
                          # ^^^ def: arr
    arr.reduce(0, :+)
  # ^^^ usage: arr
  end

  def method_with_optional_arg(foo = 3, bar = 5)
                             # ^^^ def: fooOpt 1
                                   # ^ def: fooOpt 2 default-arg-value
                                      # ^^^ def: barOpt
                                            # ^ def: barOpt 2 default-arg-value
    foo + 4
  # ^^^ usage: fooOpt 1,2
    bar + 10
  # ^^^ usage: barOpt 1,2
  end

  def method_with_block_inside
    [].each do |var|
              # ^^^ def: var
      var
    # ^^^ usage: var
    end
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
