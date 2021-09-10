# typed: strict
extend T::Sig

sig {returns(T::Boolean)}
def returns_t_boolean
  false
end

sig {params(x: T::Boolean).void}
def using_and_and(x)
  result = x
  1.times do
    result &&= returns_t_boolean
  end
end

