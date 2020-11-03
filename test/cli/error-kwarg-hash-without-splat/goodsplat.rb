# typed: true

extend T::Sig

sig {params(x: Integer, y:Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}
takes_kwargs(99, **arghash)
