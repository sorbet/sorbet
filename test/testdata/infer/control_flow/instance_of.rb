# typed: true

class A; end;
class B; end;
class C; end;

extend T::Sig

sig {params(x: T.any(A, B, C)).void}
def baz(x)
  if x.instance_of?(A)
    T.assert_type!(x, A)
  end
  if !x.instance_of?(A)
    T.assert_type!(x, T.any(B, C))
  end
end
