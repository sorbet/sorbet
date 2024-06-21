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

class I; end
class S; end
class A
  extend T::Sig
  sig {params(x: I).void}
  sig {params(x: S).void}
  def my_method(x); end # error: against an overloaded signature
end


A.new.my_metho # error: does not exist
#             ^ completion: my_method, my_method
