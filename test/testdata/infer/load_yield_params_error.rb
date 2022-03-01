# typed: strict

extend T::Sig

sig {params(xs: T.nilable(T::Array[String])).returns(T.untyped)}
def call(xs)
  xs.map do |x|
  #  ^^^ error: Method `map` does not exist on `NilClass` component of `T.nilable(T::Array[String])`
    T.reveal_type(x) # error: `String`
  end
end
