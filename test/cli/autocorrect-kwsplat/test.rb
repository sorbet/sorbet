# typed: strict
extend T::Sig

sig {params(pos: Integer, x: Integer).void}
def requires_x(*pos, x:)
end

sig {params(pos: Integer, x: String).void}
def optional_x(*pos, x: '')
end

opts = T::Hash[Symbol, Integer].new
string_keys = T::Hash[String, Integer].new
nilble_opts = T::Hash[Symbol, T.nilable(Integer)].new
x = T.let(:x, Symbol)

requires_x(0, 1, 2, opts)

requires_x(0, 1, 2, **opts)

requires_x(0, 1, 2, **opts, x: 0)

requires_x(0, 1, 2, x: 0, **opts)

requires_x(0, 1, 2, **{x => 0})

optional_x(0, 1, 2, **string_keys)

optional_x(0, 1, 2, **nilble_opts)
