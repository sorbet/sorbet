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
  # TODO(jez) Assert that we get the right redefinition based on the sig.
  a.some_ # error: does not exist
#        ^ completion: some_method
end
