# typed: true

class A
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_a; end
end
class B
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_b; end
end

# This test exists to document behavior that is currently broken and should be
# fixed. We shouldn't suggest `foo_only_on_a`.

def test
  ab = T.let(A.new, T.any(A, B))
  ab.foo_
#        ^ completion: foo_common_1, foo_common_2, foo_only_on_a
# ^^^^^^^ error: Method `foo_` does not exist on `A` component of `T.any(A, B)`
# ^^^^^^^ error: Method `foo_` does not exist on `B` component of `T.any(A, B)`
end
