# typed: true

# This test shows off an exception to our slow path decision logic: when we receive
# a lot of updates that don't actually change the state of the indexer's file table,
# we skip applying those updates to shrink the edit down. The result is that we may
# still be able to take the fast path, despite seeing a large number of change
# notifications.

class MyClass
  def my_method
    a = T.let(10, String) # error: Argument does not have asserted type `String`
  end
end
