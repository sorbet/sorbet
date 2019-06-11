# typed: true

class A; end
class B; end
class C; end

extend T::Sig

# TODO(azdavis) This should not be an error after exhaustiveness checks
sig {params(x: T.any(A, B, C)).returns(T.any(A, B, C))}
def foo(x) # error: Returning value that does not conform to method result type
  case x
  when A, B # error: Revealed type: `T.any(B, A)`
    T.reveal_type(x)
  when C
    T.reveal_type(x) # error: Revealed type: `C`
  end
end
