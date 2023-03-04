# typed: true
# experimental-ruby3-keyword-args: true

extend T::Sig

sig {params(x: Integer, y: Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}
takes_kwargs(99, arghash) # error: Keyword argument hash without `**` is deprecated

takes_kwargs(99, **arghash)

sig do
  params(
    name: String,
    tags: T::Hash[T.untyped, T.untyped],
    x: Integer,
    blk: T.proc.returns(String)
  )
  .returns(String)
end
def foo(name, tags: {}, x: 42, &blk)
  yield
end

blk = Proc.new
foo("", tags: {}, x: 1, &blk)


def with_base_dir(*segments)
end

Dir[with_base_dir("")].each do |file_path|
  p file_path
end


# Carried over from test/cli/autocorrect-kwsplat/test.rb
# For more details see https://github.com/sorbet/sorbet/pull/6771
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
                  # ^^^^ error: Keyword argument hash without `**` is deprecated
requires_x(0, 1, 2, **opts)

requires_x(0, 1, 2, **opts, x: 0)

requires_x(0, 1, 2, x: 0, **opts)

requires_x(0, 1, 2, **{x => 0})

optional_x(0, 1, 2, **string_keys)
                  # ^^^^^^^^^^^^^ error: Expected `Symbol` but found `String` for keyword splat keys type
optional_x(0, 1, 2, **nilble_opts)
                  # ^^^^^^^^^^^^^ error: Expected `String` for keyword parameter `x` but found `T.nilable(Integer)` from keyword splat

