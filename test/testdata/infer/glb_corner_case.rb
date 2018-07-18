# typed: true

class A; end;
class B < A; end;
class C; end;

sig(x: T.any(B, C)).void
def baz(x)
  if x.is_a?(A)
    T.assert_type!(x, B)
  end
end