# typed: strict
extend T::Sig

sig {params(x: Integer).void}
def takes_no_block(x: 0)
end

takes_no_block(x: 0) do end
#                    ^^^^^^ error: does not take a block
takes_no_block do end
#              ^^^^^^ error: does not take a block
takes_no_block{}
#             ^^ error: does not take a block

sig {params(xs: T::Array[Integer]).void}
def example1(xs)
  xs[0] {}
  #     ^^ error: does not take a block
end

f = ->(){}
takes_no_block(&f)
#              ^^ error: does not take a block

xs = []
takes_no_block(*xs, &f)
#                   ^^ error: does not take a block
