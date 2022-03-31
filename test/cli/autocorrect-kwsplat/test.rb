# typed: strict
extend T::Sig

sig {params(pos: Integer, x: Integer).void}
def foo(*pos, x:)
end

opts = T::Hash[Symbol, Integer].new
x = T.let(:x, Symbol)

foo(0, 1, 2, opts)

foo(0, 1, 2, **opts)

foo(0, 1, 2, **opts, x: 0)

foo(0, 1, 2, x: 0, **opts)

foo(0, 1, 2, **{x => 0})
