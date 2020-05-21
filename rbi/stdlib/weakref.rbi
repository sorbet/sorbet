# typed: __STDLIB_INTERNAL

# Weak Reference class that allows a referenced object to be garbage-collected.
#
# A [`WeakRef`](https://docs.ruby-lang.org/en/2.6.0/WeakRef.html) may be used
# exactly like the object it references.
#
# Usage:
#
# ```ruby
# foo = Object.new            # create a new object instance
# p foo.to_s                  # original's class
# foo = WeakRef.new(foo)      # reassign foo with WeakRef instance
# p foo.to_s                  # should be same class
# GC.start                    # start the garbage collector
# p foo.to_s                  # should raise exception (recycled)
# ```
class WeakRef < Delegator
  # Creates a weak reference to `orig`
  #
  # Raises an ArgumentError if the given `orig` is immutable, such as
  # [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html),
  # [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html), or
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html).
  def self.new(orig); end

  # Returns true if the referenced object is still alive.
  def weakref_alive?; end
end

class WeakRef::ObjectSpace; end

# [`RefError`](https://docs.ruby-lang.org/en/2.6.0/WeakRef/RefError.html) is
# raised when a referenced object has been recycled by the garbage collector
class WeakRef::RefError < ::StandardError; end
