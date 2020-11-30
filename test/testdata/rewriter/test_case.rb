# typed: true

class MyTest < ActiveSupport::TestCase
  # Test cases where rewriter shouldn't modify the code
  tesst "invalid", "method name" do
  end

  test "invalid", "parameter count" do
  end

  test "no block argument"

  test :not_a_string do
  end
end