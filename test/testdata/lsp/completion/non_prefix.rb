# typed: true
extend T::Sig

sig {params(x: T::Array[Integer]).void}
def foo(x)
  x.me # error: does not exist
  #   ^ completion: member?, method, methods, Pathname, __method__, define_singleton_method, ...
end
