# typed: true
module ActiveSupport::TestCase
end

class MyTest < ActiveSupport::TestCase
  # Helper instance method
  def assert(test)
    test ? true : false
  end

  # Helper method to direct calls to `test` instead of Kernel#test
  def self.test(*args)
  end

  # Method calls that shouldn't be rewritten
  tesst "invalid", "method name" do # error: Method `tesst` does not exist on `T.class_of(MyTest)`
  end

  test "invalid", "parameter count" do
  end

  test "no block argument"

  test :not_a_string do
  end

  test :not_a_string do
    assert true # error: Method `assert` does not exist on `T.class_of(MyTest)`
  end
  # end unmodified method calls

  test "valid method call" do
  end

  test "block is evaluated in the context of an instance" do
    assert true
  end
end
