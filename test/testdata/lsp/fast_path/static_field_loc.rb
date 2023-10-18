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
# This is a rather large comment at the start of the file so that when we
# delete it on the fast path, it will make it very likely that we have an array
# access that's out of bounds when reporting errors.
#
class B
  extend T::Generic
  R = 1
  R = type_member
# ^ error: Redefining constant `R` as a type member or type template
end
