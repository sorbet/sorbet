# typed: true

# Sorbet does not take the fast path when it would have to check a "large" number of files (10 in the test suite).
# Changing `my_method` implicates this file and the 9 files that call it. This test ensures that the changed
# `typed: ignore` file is not counted as an 11th file, as there is nothing in it to check.

class MyClass
  extend T::Sig

  sig { returns(Integer) }
  def self.my_method
    0
  end
end
