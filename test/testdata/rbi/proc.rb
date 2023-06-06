# typed: strict
extend T::Sig

-> (x) { x }[123]

my_proc = Proc.new {}
T.reveal_type(my_proc) # error: type: `Proc`

sig {params(f: T.proc.params(x: Integer).void).void}
def example(f)
  f.call(0)
end

f = ->(x, &blk) do
  puts x
  blk.call
end

example(f)
#       ^ error: Expected `T.proc.params(arg0: Integer).void` but found `T.proc.params(arg0: T.untyped, arg1: T.untyped).returns(T.untyped)` for argument `f`
