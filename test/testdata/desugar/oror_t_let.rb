# typed: true
extend T::Sig

def returns_untyped; end

sig {params(customer: T.nilable(Integer)).returns(Integer)}
def call(customer)
  customer ||= T.let(returns_untyped, Integer)
  T.reveal_type(customer) # error: Revealed type: `Integer`
  customer
end

