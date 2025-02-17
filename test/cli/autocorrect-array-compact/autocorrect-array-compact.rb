# typed: true
extend T::Sig

sig {params(xs: T::Array[Integer]).returns(T::Array[Integer])}
def takes_int_list(xs)
  result = xs.concat([nil])
  result.compact
end

sig {params(xs: T::Array[T.nilable(Integer)]).void}
def takes_nilable_int_list(xs)
  takes_int_list(xs)
end
