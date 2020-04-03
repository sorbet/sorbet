# typed: true

# This test used to crash the DefaultArgs pass because it tried to dereference
# the block pointer to get it's body, but the block itself was nullptr.

sig # error: `sig` requires a block parameter
def foo(x: nil)
end
