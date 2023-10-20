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

module Parent
  extend T::Generic
  has_attached_class! { {upper: Integer} }
end

class Child
  extend T::Generic
  extend Parent
end

# module Parent
#   extend T::Generic
#   FakeAttachedClass = type_member { {upper: Integer} }
# end

# class Child
#   extend T::Generic
#   extend Parent
#   FakeAttachedClass = type_template { {upper: Child} }
# end
