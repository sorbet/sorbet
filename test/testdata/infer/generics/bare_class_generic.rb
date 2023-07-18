# typed: true
extend T::Sig

sig {params(klass: Class).void}
def example(klass)
  T.reveal_type(klass) # error: `T::Class[T.anything]`
  instance = klass.new
  T.reveal_type(instance) # error: `T.anything`
  instance.foo # error: Method `foo` does not exist on `T.anything`
end
