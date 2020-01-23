# frozen_string_literal: true
# typed: true
# compiled: true
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
