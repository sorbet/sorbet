# typed: true
extend T::Sig

sig {params(x: T.nilable(Integer)).void}
def example1(x)
  T.must(x)
  T.reveal_type(x) # error: `Integer`
end

sig {params(x: T.nilable(T::Boolean)).void}
def example2(x)
  T.must(x)
  T.reveal_type(x) # error: `T::Boolean`
end
