# typed: true
extend T::Sig

sig {
  type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
}
def takes_block(&blk)
  yield
end

sig {params("&": T.proc.returns(Integer)).void}
def foo(&)
  res = takes_block(&)
  T.reveal_type(res) # error: `Integer`
end
