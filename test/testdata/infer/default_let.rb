# typed: true

extend T::Sig

sig {params(x: T.untyped).void}
def foo(x = nil)
end

sig {params(x: Integer).void}
def bar(x = Integer.new)
end
