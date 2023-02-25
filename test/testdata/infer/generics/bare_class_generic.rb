# typed: true
extend T::Sig

sig {params(klass: Class).void}
def example(klass)
  T.reveal_type(klass)
  instance = klass.new
  T.reveal_type(instance)
  instance.foo
end
