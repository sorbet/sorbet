# typed: true

# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.
# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.
# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.
# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.
# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.

class Parent
  extend T::Helpers
  sealed!
end

class Child < Parent
  extend T::Sig

  sig { override.returns(Integer) }
  def self.sealed_subclasses
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Return type `Integer` does not match return type of overridden method `Parent.sealed_subclasses`
    0
  end
end
