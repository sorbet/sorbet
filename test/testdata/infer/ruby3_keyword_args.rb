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

blk = Proc.new {}
foo("", tags: {}, x: 1, &blk)
