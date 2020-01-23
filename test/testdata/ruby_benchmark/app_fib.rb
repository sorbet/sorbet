# frozen_string_literal: true
# typed: strict
# compiled: true

class HasFib
  T::Sig::WithoutRuntime.sig(:final) {params(n: Integer).returns(Integer)}
  def self.fib(n)
    if n < 3
      1
    else
      fib(n-1) + fib(n-2)
    end
  end
end

HasFib.fib(34)

