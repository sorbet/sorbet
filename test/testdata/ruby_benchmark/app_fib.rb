# typed: strict

class HasFib
  T::Sig::WithoutRuntime.sig{params(n: Integer).returns(Integer)}
  def self.fib(n)
    if n < 3
      1
    else
      fib(n-1) + fib(n-2)
    end
  end
end

HasFib.fib(34)

