# typed: true

# Test that 'it' block parameter can coexist with RSpec's 'it' method

class MyTest
  extend T::Sig

  # RSpec DSL method 'it'
  it "processes items" do
    # Using 'it' as a block parameter within RSpec's 'it' block
    [1, 2, 3].map { it * 2 }
  end

  # Multiple levels of 'it' usage
  it "handles nested blocks" do
    # Outer block uses 'it' parameter
    [[1, 2], [3, 4]].map { it.sum }

    # Inner block also uses 'it' parameter (shadowing outer)
    [5, 6, 7].each { puts it }
  end

  # Using 'it' as both method name and block parameter
  it "uses it in lambda" do
    lambda { it + 1 }.call(5)
  end
end
