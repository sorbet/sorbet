# typed: true
extend T::Sig
  sig {params(x: Integer); returns(Integer)}
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: No return type specified.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Signature blocks must contain a single statement
def foo(x)
  x
end
foo(3)
