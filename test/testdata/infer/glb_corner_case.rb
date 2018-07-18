# typed: true

class A; end;
class B < A; end;
class C; end;

sig(x: T.any(B, C)).void
def baz(x)
  if x.is_a?(A)
    d = T.let(x, B)
    puts d
  end
end