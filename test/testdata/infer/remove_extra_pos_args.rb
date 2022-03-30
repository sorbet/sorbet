# typed: true
extend T::Sig

sig {params(x: Integer, blk: T.nilable(T.proc.void)).void}
def takes_one_arg(x, &blk)
end

takes_one_arg(0)
takes_one_arg(0, 0)
#                ^ error: Too many arguments
takes_one_arg(0, 0, 0)
#                ^^^^  error: Too many arguments

takes_one_arg(0) do
  nil
end
takes_one_arg(0, 0) do
  #              ^ error: Too many arguments
  nil
end
takes_one_arg(0, 0, 0) do
  #              ^^^^ error: Too many arguments
  nil
end
