# typed: true

# This test ensures that we _always_ take the slow path when a "large" number of files change -- even when the changes
# could technically take the fast path. This is a performance optimization, as parallelism is only available on the
# slow path.
# The other files in this test are empty. This file has code to ensure that errors are properly updated.

class MyClass
  def my_method
    a = T.let(10, String) # error: Argument does not have asserted type `String`
  end
end
