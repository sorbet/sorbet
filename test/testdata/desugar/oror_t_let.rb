# typed: true
extend T::Sig

def returns_untyped; end

sig {params(customer: T.nilable(Integer)).returns(T.untyped)}
def call(customer)
  customer ||= T.let(returns_untyped, Integer)
  customer
end

