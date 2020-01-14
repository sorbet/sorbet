# typed: true

class A
  # Imagine that this class exists somewhere else in a `typed: false` file.
  # That is, this situation is definitely one that could occur in real code.
  def some_method; end
  def some_method(x); end # error: redefined
end

# We should pretty much always ignore mangled names that result from method
# redefinition for the purposes of completion items.

def test
  a = A.new
  # This doesn't test that we got the *right* overload, just that we aren't
  # sending back two suggestions with the same name, which is ~good enough.
  a.some_ # error: does not exist
#        ^ completion: some_method
end
