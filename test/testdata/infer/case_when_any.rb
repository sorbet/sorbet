# typed: true

class A; end
class B; end
class C; end

extend T::Sig

sig {params(x: T.any(A, B, C)).returns(T.any(A, B, C))}
def foo(x)
  case x
  when A, B
    T.reveal_type(x) # error: Revealed type: `T.any(B, A)`
    x
  when C
    T.reveal_type(x) # error: Revealed type: `C`
    x
  end
end
