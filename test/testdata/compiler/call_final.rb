# frozen_string_literal: true
# typed: true
# compiled: true

class A; 
  T::Sig::WithoutRuntime.sig(:final) {params(n: Integer).returns(Integer)}
  def self.foo(n)
    n
  end
end;
class B
def caller(a)
  A.foo(a)
end
end

# These groups of assertions check that the call to the direct wrapper is
# present in the un-optimized ir, and that it gets inlined in the optimized
# version.


