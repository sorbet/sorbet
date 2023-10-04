# typed: strong
extend T::Sig

sig {params(x: T::Hash[T.untyped, T::Hash[T.untyped, T.untyped]]).void}
def example1(x)
  x["foo"]["bar"]
  #       ^^^^^^^ error: Method `[]` does not exist on `NilClass` component
end

sig {params(x: T.nilable(T::Hash[T.untyped, T::Hash[T.untyped, T.untyped]])).void}
def example2(x)
  x["foo"]["bar"]
  #^^^^^^^ error: Method `[]` does not exist on `NilClass` component
  #       ^^^^^^^ error: Call to method `[]` on `T.untyped`

end


