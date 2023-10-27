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

class Alias
  def foo; end

  alias_method :method_with_specific_name, :foo
end

Alias.new.method_with_specific_nam
#         ^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `method_with_specific_nam` does not exist on `Alias`
